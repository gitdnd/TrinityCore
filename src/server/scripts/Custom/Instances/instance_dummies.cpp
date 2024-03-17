#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Map.h"

class instance_kara_crypts : public InstanceMapScript
{
public:
    instance_kara_crypts() : InstanceMapScript("instance_kara_crypts", 760) { }

    struct instance_kara_crypts_InstanceMapScript : public InstanceScript
    {
        instance_kara_crypts_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
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

class instance_fall_of_dalaran : public InstanceMapScript
{
public:
    instance_fall_of_dalaran() : InstanceMapScript("instance_fall_of_dalaran", 560) { }

    struct instance_fall_of_dalaran_InstanceMapScript : public InstanceScript
    {
        instance_fall_of_dalaran_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("FD");
            SetBossNumber(5);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_fall_of_dalaran_InstanceMapScript(map);
    }
};

class instance_northshire_siege : public InstanceMapScript
{
public:
    instance_northshire_siege() : InstanceMapScript("instance_northshire_siege", 762) { }

    struct instance_northshire_siege_InstanceMapScript : public InstanceScript
    {
        instance_northshire_siege_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("NS");
            SetBossNumber(3);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_northshire_siege_InstanceMapScript(map);
    }
};

class instance_valour_keep : public InstanceMapScript
{
public:
    instance_valour_keep() : InstanceMapScript("instance_valour_keep", 763) { }

    struct instance_valour_keep_InstanceMapScript : public InstanceScript
    {
        instance_valour_keep_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("VK");
            SetBossNumber(4);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_valour_keep_InstanceMapScript(map);
    }
};

class instance_worlds_end : public InstanceMapScript
{
public:
    instance_worlds_end() : InstanceMapScript("instance_worlds_end", 309) { }

    struct instance_worlds_end_InstanceMapScript : public InstanceScript
    {
        instance_worlds_end_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("WE");
            SetBossNumber(4);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_worlds_end_InstanceMapScript(map);
    }
};

class instance_stromgarde : public InstanceMapScript
{
public:
    instance_stromgarde() : InstanceMapScript("instance_stromgarde", 309) { }

    struct instance_stromgarde_InstanceMapScript : public InstanceScript
    {
        instance_stromgarde_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("SG");
            SetBossNumber(7);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_stromgarde_InstanceMapScript(map);
    }
};

class instance_arathor : public InstanceMapScript
{
public:
    instance_arathor() : InstanceMapScript("instance_arathor", 759) { }

    struct instance_arathor_InstanceMapScript : public InstanceScript
    {
        instance_arathor_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("AR");
            SetBossNumber(3);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_arathor_InstanceMapScript(map);
    }
};

void AddSC_Instance_Dummies()
{
    new instance_kara_crypts();
    new instance_fall_of_dalaran();
    new instance_northshire_siege();
    new instance_valour_keep();
    new instance_worlds_end();
    new instance_stromgarde();
    new instance_arathor();
}
