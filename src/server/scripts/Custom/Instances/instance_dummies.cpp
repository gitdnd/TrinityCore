#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Map.h"

class instance_kara_crypts : public InstanceMapScript
{
public:
    instance_kara_crypts() : InstanceMapScript("instance_kara_crypts", 760) { }

    struct instance_kara_crypts_InstanceMapScript : public InstanceScript
    {
        instance_kara_crypts_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetHeaders("KC");
            SetBossNumber(4);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_kara_crypts_InstanceMapScript(map);
    }
};

void AddSC_Instance_Dummies()
{
    new instance_kara_crypts();
}
