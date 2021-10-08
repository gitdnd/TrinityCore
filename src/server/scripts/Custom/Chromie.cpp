#include "ScriptMgr.h"
#include "Player.h"
#include "WorldSession.h"
class chromie_script : public PlayerScript
{
public:
    chromie_script() : PlayerScript("chromie_script") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        if (player->GetSession()->GetAccountId() == 638)
        {
            player->SetNativeDisplayId(24877);
            player->SetDisplayId(24877);
        }

    }
};

void AddSC_chromie_script()
{
    new chromie_script();
}
