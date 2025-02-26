/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "GameTime.h"
#include "World.h"
#include "Util.h"
#include "Warden.h"
#include "AccountMgr.h"
#include "WardenPackets.h"

#include <openssl/sha.h>

Warden::Warden() : _session(nullptr), _checkTimer(10000/*10 sec*/), _clientResponseTimer(0),
                   _dataSent(false), _previousTimestamp(0), _module(nullptr), _initialized(false)
{
    memset(_inputKey, 0, sizeof(_inputKey));
    memset(_outputKey, 0, sizeof(_outputKey));
    memset(_seed, 0, sizeof(_seed));
}

Warden::~Warden()
{
    delete[] _module->CompressedData;
    delete _module;
    _module = nullptr;
    _initialized = false;
}

void Warden::SendModuleToClient()
{
    TC_LOG_DEBUG("warden", "Send module to client");

    // Create packet structure
    WardenModuleTransfer packet;

    uint32 sizeLeft = _module->CompressedSize;
    uint32 pos = 0;
    uint16 burstSize;
    while (sizeLeft > 0)
    {
        burstSize = sizeLeft < 500 ? sizeLeft : 500;
        packet.Command = WARDEN_SMSG_MODULE_CACHE;
        packet.DataSize = burstSize;
        memcpy(packet.Data, &_module->CompressedData[pos], burstSize);
        sizeLeft -= burstSize;
        pos += burstSize;

        EncryptData((uint8*)&packet, burstSize + 3);
        WorldPacket pkt1(SMSG_WARDEN3_DATA, burstSize + 3);
        pkt1.append((uint8*)&packet, burstSize + 3);
        _session->SendPacket(&pkt1);
    }
}

void Warden::RequestModule()
{
    TC_LOG_DEBUG("warden", "Request module");

    // Create packet structure
    WardenModuleUse request;
    request.Command = WARDEN_SMSG_MODULE_USE;

    memcpy(request.ModuleId, _module->Id, 16);
    memcpy(request.ModuleKey, _module->Key, 16);
    request.Size = _module->CompressedSize;

    // Encrypt with warden RC4 key.
    EncryptData((uint8*)&request, sizeof(WardenModuleUse));

    WorldPacket pkt(SMSG_WARDEN3_DATA, sizeof(WardenModuleUse));
    pkt.append((uint8*)&request, sizeof(WardenModuleUse));
    _session->SendPacket(&pkt);
}

void Warden::Update()
{
    if (_initialized)
    {
        uint32 currentTimestamp = GameTime::GetGameTimeMS();
        uint32 diff = currentTimestamp - _previousTimestamp;
        _previousTimestamp = currentTimestamp;

        if (_dataSent)
        {
            uint32 maxClientResponseDelay = sWorld->getIntConfig(CONFIG_WARDEN_CLIENT_RESPONSE_DELAY);

            if (maxClientResponseDelay > 0)
            {
                // Kick player if client response delays more than set in config
                if (_clientResponseTimer > maxClientResponseDelay * IN_MILLISECONDS)
                {
                    TC_LOG_WARN("warden", "%s (latency: %u, IP: %s) exceeded Warden module response delay for more than %s - disconnecting client",
                                   _session->GetPlayerInfo().c_str(), _session->GetLatency(), _session->GetRemoteAddress().c_str(), secsToTimeString(maxClientResponseDelay, TimeFormat::ShortText).c_str());
                    _session->KickPlayer("Warden::Update Warden module response delay exceeded");
                }
                else
                    _clientResponseTimer += diff;
            }
        }
        else
        {
            if (diff >= _checkTimer)
            {
                RequestData();
            }
            else
                _checkTimer -= diff;
        }
    }
}

void Warden::DecryptData(uint8* buffer, uint32 length)
{
    _inputCrypto.UpdateData(buffer, length);
}

void Warden::EncryptData(uint8* buffer, uint32 length)
{
    _outputCrypto.UpdateData(buffer, length);
}

bool Warden::IsValidCheckSum(uint32 checksum, uint8 const* data, const uint16 length)
{
    uint32 newChecksum = BuildChecksum(data, length);

    if (checksum != newChecksum)
    {
        TC_LOG_DEBUG("warden", "CHECKSUM IS NOT VALID");
        return false;
    }
    else
    {
        TC_LOG_DEBUG("warden", "CHECKSUM IS VALID");
        return true;
    }
}

struct keyData {
    union
    {
        struct
        {
            uint8 bytes[20];
        } bytes;

        struct
        {
            uint32 ints[5];
        } ints;
    };
};

uint32 Warden::BuildChecksum(uint8 const* data, uint32 length)
{
    keyData hash;
    SHA1(data, length, hash.bytes.bytes);
    uint32 checkSum = 0;
    for (uint8 i = 0; i < 5; ++i)
        checkSum = checkSum ^ hash.ints.ints[i];

    return checkSum;
}

std::string Warden::Penalty(WardenCheck* check /*= nullptr*/)
{
    WardenActions action;

    if (check)
        action = check->Action;
    else
        action = WardenActions(sWorld->getIntConfig(CONFIG_WARDEN_CLIENT_FAIL_ACTION));

    switch (action)
    {
        case WARDEN_ACTION_LOG:
            return "None";
            break;
        case WARDEN_ACTION_KICK:
            _session->KickPlayer("Warden::Penalty");
            return "Kick";
            break;
        case WARDEN_ACTION_BAN:
        {
            std::string accountName;
            AccountMgr::GetName(_session->GetAccountId(), accountName);
            std::string banReason = "Warden Anticheat Violation";
            // Check can be NULL, for example if the client sent a wrong signature in the warden packet (CHECKSUM FAIL)
            if (check)
                banReason += Trinity::StringFormat(": %s (CheckId: %u", check->Comment, uint32(check->CheckId));

            sWorld->BanAccount(BAN_ACCOUNT, accountName, Trinity::StringFormat("%ds", sWorld->getIntConfig(CONFIG_WARDEN_CLIENT_BAN_DURATION)), banReason, "Server");

            return "Ban";
        }
        default:
            break;
    }
    return "Undefined";
}

void WorldSession::HandleWardenData(WorldPackets::Warden::WardenData& packet)
{
    if (!_warden || packet.Data.empty())
        return;

    _warden->DecryptData(packet.Data.contents(), packet.Data.size());
    uint8 opcode;
    packet.Data >> opcode;
    TC_LOG_DEBUG("warden", "Got packet, opcode %02X, size %u", opcode, uint32(packet.Data.size()));
    packet.Data.hexlike();

    switch (opcode)
    {
        case WARDEN_CMSG_MODULE_MISSING:
            _warden->SendModuleToClient();
            break;
        case WARDEN_CMSG_MODULE_OK:
            _warden->RequestHash();
            break;
        case WARDEN_CMSG_CHEAT_CHECKS_RESULT:
            _warden->HandleData(packet.Data);
            break;
        case WARDEN_CMSG_MEM_CHECKS_RESULT:
            TC_LOG_DEBUG("warden", "NYI WARDEN_CMSG_MEM_CHECKS_RESULT received!");
            break;
        case WARDEN_CMSG_HASH_RESULT:
            _warden->HandleHashResult(packet.Data);
            _warden->InitializeModule();
            break;
        case WARDEN_CMSG_MODULE_FAILED:
            TC_LOG_DEBUG("warden", "NYI WARDEN_CMSG_MODULE_FAILED received!");
            break;
        default:
            TC_LOG_DEBUG("warden", "Got unknown warden opcode %02X of size %u.", opcode, uint32(packet.Data.size() - 1));
            break;
    }
}
