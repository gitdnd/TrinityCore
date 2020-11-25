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
#include <mutex>
#include <limits>
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

constexpr size_t PATCH_BUFFER_MAX_SIZE = 4096;

typedef struct PATCH_INFO
{
public:
    int build;
    int locale;
    uint64 filesize;
    uint8 md5[MD5_DIGEST_LENGTH];

    std::vector<ByteBuffer*>& GetBuffers() { return _buffers; }
    const std::vector<ByteBuffer*>& GetBuffers() { return _buffers; } const
    const ByteBuffer* GetBuffer(uint16 index) const
    {
        assert(index < _buffers.size());
        return _buffers[index];
    }

private:
    std::vector<ByteBuffer*> _buffers;
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

    const PATCH_INFO& GetPatchInfo(uint16 index)
    {
        assert(index < _patches.size());
        return _patches[index];
    }

private:
    PATCH_INFO* getPatchInfo(int _build, std::string _locale, bool* fallback, uint16& patchInfoIndex);
    void LoadPatchesInfo();
    Patches _patches;
    std::string m_dataDir;
};

// A PatchSession is a single instance of a session between the Patcher and a client. This session will transfer the patch data from the patcher to the client
struct PatchSession
{
    AuthSession* session = nullptr;
    uint16 patchIndex =  std::numeric_limits<uint16>().max();  // This is the index into our patch info array.
    uint16 bufferIndex = std::numeric_limits<uint16>().max(); // This is the index into our prebuilt array of buffers for the patch info we are sending.
};

class PatcherService
{
public:
    PatcherService() {}

    void Start()
    {
        assert(_isRunning == false);

        _isRunning = true;
        _thread = new boost::thread(&PatcherService::Run, this);
        _thread->detach();
    }
    void Stop()
    {
        assert(_isRunning == true);
        _isRunning = false;

        delete _thread;
    }
    void Run();

    // FNV-1a 32bit hashing algorithm.
    uint32 fnv1a_32(const char* s, std::size_t count)
    {
        return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
    }

    std::unordered_map<uint32, PatchSession>& GetPatchSessions() { return _patchSessions; }
    std::mutex& GetMutex() { return _mutex; }
    boost::thread*& GetThread() { return _thread; }
    bool IsRunning() { return _isRunning; }

private:
    std::unordered_map<uint32, PatchSession> _patchSessions;

    std::mutex _mutex;
    boost::thread* _thread;
    bool _isRunning = false;
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

    void SendPacket(const ByteBuffer& packet);
    void SetPatchInfoIndex(const uint16 patchInfoIndex) { _patchInfoIndex = patchInfoIndex;}

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

    bool VerifyVersion(const uint8* a, int32 aLength, const uint8* versionProof, bool isReconnect);

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
    uint16 _patchInfoIndex = std::numeric_limits<uint16>().max();
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
