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

#ifndef __AUTHSESSION_H__
#define __AUTHSESSION_H__

#include "AsyncCallbackProcessor.h"
#include "BigNumber.h"
#include "ByteBuffer.h"
#include "Common.h"
#include "Optional.h"
#include "Socket.h"
#include "QueryResult.h"
#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <openssl/md5.h>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class AuthSession;
class Field;
struct AuthHandler;

enum AuthStatus
{
    STATUS_CHALLENGE = 0,
    STATUS_LOGON_PROOF,
    STATUS_RECONNECT_PROOF,
    STATUS_AUTHED,
    STATUS_WAITING_FOR_REALM_LIST,
    STATUS_CLOSED
};

// TODO: Add as config variable.
#ifndef _WIN32
 #define PATCH_PATH "../var/patches/"
#else
 #define PATCH_PATH "./patches/"
#endif

typedef struct PATCH_INFO
{
    int build;
    int locale;
    uint64 filesize;
    uint8 md5[MD5_DIGEST_LENGTH];
} PATCH_INFO;

class Patcher
{
    typedef std::vector<PATCH_INFO> Patches;
public:
    void Initialize();

    void LoadPatchMD5(const char*, char*);
    bool GetHash(char* pat, uint8 mymd5[16]);

    bool InitPatching(int _build, std::string _locale, AuthSession* _session);
    bool PossiblePatching(int _build, std::string _locale);

private:
    PATCH_INFO* getPatchInfo(int _build, std::string _locale, bool* fallback);
    void LoadPatchesInfo();
    Patches _patches;
    std::string m_dataDir;
};

// Launch a thread to transfer a patch to the client
class PatcherRunnable
{
public:
    PatcherRunnable(AuthSession* session, uint64 start, uint64 size);
    void run();
    void stop();
    boost::thread* patchThread;
private:
    AuthSession* mySocket;
    uint64 pos;
    uint64 size;
    bool stopped;
};

struct AccountInfo
{
    void LoadResult(Field* fields);

    uint32 Id = 0;
    std::string Login;
    bool IsLockedToIP = false;
    std::string LockCountry;
    std::string LastIP;
    uint32 FailedLogins = 0;
    bool IsBanned = false;
    bool IsPermanenetlyBanned = false;
    AccountTypes SecurityLevel = SEC_PLAYER;
};

class AuthSession : public Socket<AuthSession>
{
    typedef Socket<AuthSession> AuthSocket;

public:
    static std::unordered_map<uint8, AuthHandler> InitHandlers();

    AuthSession(tcp::socket&& socket);

    void Start() override;
    bool Update() override;

    void SendPacket(ByteBuffer& packet);

    FILE* pPatch;
    PatcherRunnable* _patcher;

protected:
    void ReadHandler() override;

private:
    bool HandleLogonChallenge();
    bool HandleLogonProof();
    bool HandleReconnectChallenge();
    bool HandleReconnectProof();
    bool HandleRealmList();
    bool HandleXferAccept();
    bool HandleXferResume();
    bool HandleXferCancel();

    void CheckIpCallback(PreparedQueryResult result);
    void LogonChallengeCallback(PreparedQueryResult result);
    void ReconnectChallengeCallback(PreparedQueryResult result);
    void RealmListCallback(PreparedQueryResult result);

    void SetVSFields(const std::string& rI);

    bool VerifyVersion(uint8 const* a, int32 aLength, uint8 const* versionProof, bool isReconnect);

    BigNumber N, s, g, v;
    BigNumber b, B;
    BigNumber K;
    BigNumber _reconnectProof;

    AuthStatus _status;
    AccountInfo _accountInfo;
    Optional<std::vector<uint8>> _totpSecret;
    std::string _localizationName;
    std::string _os;
    std::string _ipCountry;
    uint16 _build;
    uint8 _expversion;

    QueryCallbackProcessor _queryProcessor;
};

#pragma pack(push, 1)

struct AuthHandler
{
    AuthStatus status;
    size_t packetSize;
    bool (AuthSession::*handler)();
};

#pragma pack(pop)

#endif
