
#include "AccountMgr.h"
#include "AES.h"
#include "Base32.h"
#include "Chat.h"
#include "CryptoGenerics.h"
#include "DatabaseEnv.h"
#include "IpAddress.h"
#include "IPLocation.h"
#include "Language.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SecretMgr.h"
#include "TOTP.h"
#include "World.h"
#include "WorldSession.h"
#include <unordered_map>
#include <openssl/rand.h>
#include <boost/optional/optional_io.hpp>

using namespace Trinity::ChatCommands;

class discord_commandscript : public CommandScript
{
public:
    discord_commandscript() : CommandScript("discord_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> discordCommandTable =
        {
            { "forgotpassword",         rbac::RBAC_PERM_COMMAND_DISCORD_FORGOT_PASSWORD,          true,  &HandlDiscordForgotPasswordCommand,       ""       },
            { "forgotusername",         rbac::RBAC_PERM_COMMAND_DISCORD_FORGOT_USERNAME,          true,  &HandlDiscordForgotUsernameCommand,       ""       },
            { "changepassword",         rbac::RBAC_PERM_COMMAND_DISCORD_CHANGE_PASSWORD,          true,  &HandlDiscordChangePasswordCommand,       ""       },
            { "setup2fa",               rbac::RBAC_PERM_COMMAND_DISCORD_SETUP_2FA,                true,  &HandlDiscordSetup2FACommand,       ""       },
            { "registeraccount",        rbac::RBAC_PERM_COMMAND_DISCORD_REGISTER_ACCOUNT,         true,  &HandlDiscordRegisterAccountCommand,       ""       },
            { "registeraccesskey",      rbac::RBAC_PERM_COMMAND_DISCORD_REGISTER_ACCESS_KEY,      true,  &HandlDiscordRegisterAccessKeyCommand,       ""       },
            { "status",                 rbac::RBAC_PERM_COMMAND_DISCORD_ACCOUNT_STATUS,           true,  &HandlDiscordAccountStatusCommand,       ""       },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "discord",        rbac::RBAC_PERM_COMMAND_ACCOUNT,                 true,  nullptr,              "",  discordCommandTable },
        };
        return commandTable;
    }

    static bool HandlDiscordForgotPasswordCommand(ChatHandler* handler, std::string const& discordId)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        std::string password = "";

        for (int i = 0; i < 15; i++)
        {
            std::string passwordCharacters = "abcdefghijklmnopqrstuvwxyz1234567890";
            int random = rand() % passwordCharacters.size();
            password.append(passwordCharacters, random, 1);
        }
        AccountMgr::ChangePassword(accountId, password);

        handler->PSendSysMessage("Your new password is now %s", password);
        return true;
    }

    static bool HandlDiscordForgotUsernameCommand(ChatHandler* handler, std::string const& discordId)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        std::string username = AccountMgr::GetUsernameById(accountId);
        if (username == "") // How would this even happen
        {
            handler->PSendSysMessage("An error occured fetching your username");
            return true;
        }

        handler->PSendSysMessage("Your username is %s.", username);
        return true;
    }

    static bool HandlDiscordChangePasswordCommand(ChatHandler* handler, std::string const& discordId, std::string const& password)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
             return true;

        AccountOpResult result = AccountMgr::ChangePassword(accountId, password);
        switch (result)
        {
        case AccountOpResult::AOR_OK:
            handler->SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AccountOpResult::AOR_PASS_TOO_LONG:
            handler->SendSysMessage(LANG_PASSWORD_TOO_LONG);
            return true;
        default:
            handler->SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            return true;
        }

        return true;
    }

    static bool HandlDiscordSetup2FACommand(ChatHandler* handler, std::string const& discordId, Optional<uint32> token)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        auto const& masterKey = sSecretMgr->GetSecret(SECRET_TOTP_MASTER_KEY);
        if (!masterKey.IsAvailable())
        {
            handler->SendSysMessage(LANG_2FA_COMMANDS_NOT_SETUP);
            handler->SetSentErrorMessage(true);
            return false;
        }

        { // check if 2FA already enabled
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_TOTP_SECRET);
            stmt->setUInt32(0, accountId);
            PreparedQueryResult result = LoginDatabase.Query(stmt);

            if (!result)
            {
                TC_LOG_ERROR("misc", "Account %u not found in login database when processing .account 2fa setup command.", accountId);
                handler->SendSysMessage(LANG_UNKNOWN_ERROR);
                return true;
            }

            if (!result->Fetch()->IsNull())
            {
                handler->SendSysMessage(LANG_2FA_ALREADY_SETUP);
                return true;
            }
        }

        // store random suggested secrets
        static std::unordered_map<uint32, Trinity::Crypto::TOTP::Secret> suggestions;
        auto pair = suggestions.emplace(std::piecewise_construct, std::make_tuple(accountId), std::make_tuple(Trinity::Crypto::TOTP::RECOMMENDED_SECRET_LENGTH)); // std::vector 1-argument size_t constructor invokes resize
        if (pair.second) // no suggestion yet, generate random secret
            RAND_bytes(pair.first->second.data(), pair.first->second.size());

        if (!pair.second && token.get()) // suggestion already existed and token specified - validate
        {
            if (Trinity::Crypto::TOTP::ValidateToken(pair.first->second, *token))
            {
                if (masterKey)
                    Trinity::Crypto::AEEncryptWithRandomIV<Trinity::Crypto::AES>(pair.first->second, *masterKey);

                LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_TOTP_SECRET);
                stmt->setBinary(0, pair.first->second);
                stmt->setUInt32(1, accountId);
                LoginDatabase.Execute(stmt);
                suggestions.erase(pair.first);
                handler->SendSysMessage(LANG_2FA_SETUP_COMPLETE);
                return true;
            }
            else
            {
                //handler->SendSysMessage(LANG_2FA_INVALID_TOKEN);
                handler->PSendSysMessage("%u was invalid token.", token);
                return true;
            }
                
        }

        // new suggestion, or no token specified, output TOTP parameters
        handler->PSendSysMessage(LANG_2FA_SECRET_SUGGESTION, Trinity::Encoding::Base32::Encode(pair.first->second));
        return true;
    }

    static bool HandlDiscordRegisterAccountCommand(ChatHandler* handler, std::string const& discordId, std::string const& username, std::string const& password)
    {
        if (AccountMgr::GetIdByEmail(discordId))
        {
            handler->PSendSysMessage("You may only make 1 game account per discord account.");
            return true;
        }

        switch (sAccountMgr->CreateAccount(username, password, discordId))
        {
        case AccountOpResult::AOR_OK:
            handler->PSendSysMessage(LANG_ACCOUNT_CREATED, username);
            break;
        case AccountOpResult::AOR_NAME_TOO_LONG:
            handler->SendSysMessage(LANG_ACCOUNT_NAME_TOO_LONG);
            return true;
        case AccountOpResult::AOR_PASS_TOO_LONG:
            handler->SendSysMessage(LANG_ACCOUNT_PASS_TOO_LONG);
            return true;
        case AccountOpResult::AOR_NAME_ALREADY_EXIST:
            handler->SendSysMessage(LANG_ACCOUNT_ALREADY_EXIST);
            return true;
        case AccountOpResult::AOR_DB_INTERNAL_ERROR:
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_CREATED_SQL_ERROR, username);
            return true;
        default:
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_CREATED, username);
            return true;
        }
    }

    static bool HandlDiscordRegisterAccessKeyCommand(ChatHandler* handler, std::string const& discordId, std::string const& key)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCESS_KEY_BY_ACCOUNT);
        stmt->setString(0, key);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (!result || !result->Fetch()->IsNull())
        {
            handler->PSendSysMessage("%s is not a valid alpha key.", key);
            return true;
        }

        LoginDatabasePreparedStatement* alphaaccess = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCESS_KEY_BY_ACCOUNT);
        alphaaccess->setUInt32(0, accountId);
        alphaaccess->setString(1, key);
        LoginDatabase.Execute(alphaaccess);
        sAccountMgr->UpdateAccountAccess(nullptr, accountId, 1, -1);
        handler->PSendSysMessage("You redeemed %s key.", key);
        return true;
    }

    static bool HandlDiscordAccountStatusCommand(ChatHandler* handler, std::string const& discordId)
    {
        uint32 accountId = AccountMgr::GetIdByEmail(discordId);
        bool twoFactorEnabled = false;
        bool alphaAccess = false;

        if (accountId != 0)
        {
            LoginDatabasePreparedStatement* twofactor = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_TOTP_SECRET);
            twofactor->setUInt32(0, accountId);
            PreparedQueryResult twofactorresult = LoginDatabase.Query(twofactor);
            if (twofactorresult && !twofactorresult->Fetch()->IsNull())
                twoFactorEnabled = true;

            LoginDatabasePreparedStatement* alpha = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCESS_KEY_BY_ACCOUNT);
            alpha->setUInt32(0, accountId);
            PreparedQueryResult alpharesult = LoginDatabase.Query(alpha);
            if (alpharesult && !alpharesult->Fetch()->IsNull())
                alphaAccess = true;
        }
        handler->PSendSysMessage("%s %s %s", accountId != 0 ? "YesACC" : "noACC", twoFactorEnabled ? "Yes2FA" : "No2FA", alphaAccess ? "YesAlpha" : "NoAlpha");
    }

    static uint32 GetAccountIdByDiscordId(ChatHandler* handler,  std::string const& discordId)
    {
        uint32 accountId = AccountMgr::GetIdByEmail(discordId);

        if (!accountId)
        {
            handler->PSendSysMessage("We were unable to find an account linked to your discord account.");
            return 0;
        }

        return accountId;
    }
};

void AddSC_discord_commandscript()
{
    new discord_commandscript();
}

