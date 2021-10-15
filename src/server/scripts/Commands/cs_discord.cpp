
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
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "discord",        rbac::RBAC_PERM_COMMAND_ACCOUNT,                 true,  nullptr,              "",  discordCommandTable },
        };
        return commandTable;
    }

    static bool HandlDiscordForgotPasswordCommand(ChatHandler* handler, std::string discordId)
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

    static bool HandlDiscordForgotUsernameCommand(ChatHandler* handler, std::string discordId)
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

    static bool HandlDiscordChangePasswordCommand(ChatHandler* handler, std::string discordId, std::string password)
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

    static bool HandlDiscordSetup2FACommand(ChatHandler* handler, std::string discordId)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        return true;
    }

    static bool HandlDiscordRegisterAccountCommand(ChatHandler* handler, std::string discordId, std::string username, std::string password)
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

    static bool HandlDiscordRegisterAccessKeyCommand(ChatHandler* handler, std::string discordId, std::string key)
    {
        uint32 accountId = GetAccountIdByDiscordId(handler, discordId);
        if (!accountId)
            return true;

        return true;
    }

    static uint32 GetAccountIdByDiscordId(ChatHandler* handler,  std::string discordId)
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

