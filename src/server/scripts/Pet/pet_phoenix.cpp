
#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CombatAI.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "Pet.h"
#include "PetAI.h"
#include "ScriptedCreature.h"

enum PhoenixSpells
{
    SPELL_PHOENIX_FIREBALL = 90221,
    SPELL_FIRE_IMMUNE      = 180202
};

enum PhoenixTimers
{
    TIMER_PHEONIX_FIREBALL = 4000
};

class npc_pet_phoenix : public CreatureScript
{
public:
    npc_pet_phoenix() : CreatureScript("npc_pet_phoenix") { }

    struct npc_pet_phoenix_AI : ScriptedAI
    {
        const float CHASE_DISTANCE = 30.0f;

        npc_pet_phoenix_AI(Creature* creature) : ScriptedAI(creature) { }

        void InitializeAI() override
        {
            Unit* owner = me->GetOwner();
            if (!owner)
                return;

            owner->CastSpell(me, SPELL_FIRE_IMMUNE, true);
        }

        // custom UpdateVictim implementation to handle special target selection
        // we prioritize between things that are in combat with owner based on the owner's threat to them
        bool UpdateVictim()
        {
            Unit* owner = me->GetOwner();
            if (!owner)
                return false;

            if (!me->HasUnitState(UNIT_STATE_CASTING) && !me->IsInCombat() && !owner->IsInCombat())
                return false;

            Unit* currentTarget = me->GetVictim();
            if (currentTarget && !CanAIAttack(currentTarget))
            {
                me->InterruptNonMeleeSpells(true); // do not finish casting on invalid targets
                me->AttackStop();
                currentTarget = nullptr;
            }

            // don't reselect if we're currently casting anyway
            if (currentTarget && me->HasUnitState(UNIT_STATE_CASTING))
                return true;

            Unit* selectedTarget = nullptr;
            CombatManager const& mgr = owner->GetCombatManager();
            if (mgr.HasPvPCombat())
            { // select pvp target
                float minDistance = 0.0f;
                for (auto const& pair : mgr.GetPvPCombatRefs())
                {
                    Unit* target = pair.second->GetOther(owner);
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        continue;
                    if (!CanAIAttack(target))
                        continue;

                    float dist = owner->GetDistance(target);
                    if (!selectedTarget || dist < minDistance)
                    {
                        selectedTarget = target;
                        minDistance = dist;
                    }
                }
            }

            if (!selectedTarget)
            { // select pve target
                float maxThreat = 0.0f;
                for (auto const& pair : mgr.GetPvECombatRefs())
                {
                    Unit* target = pair.second->GetOther(owner);
                    if (!CanAIAttack(target))
                        continue;

                    float threat = target->GetThreatManager().GetThreat(owner);
                    if (threat >= maxThreat)
                    {
                        selectedTarget = target;
                        maxThreat = threat;
                    }
                }
            }

            if (!selectedTarget)
            {
                EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
                return false;
            }

            if (selectedTarget != me->GetVictim())
                AttackStartCaster(selectedTarget, CHASE_DISTANCE);
            return true;
        }

        void UpdateAI(uint32 diff) override
        {
            Unit* owner = me->GetOwner();
            if (!owner)
            {
                me->DespawnOrUnsummon();
                return;
            }

            if (_fireballTimer)
            {
                if (_fireballTimer <= diff)
                    _fireballTimer = 0;
                else
                    _fireballTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!_fireballTimer)
            {
                DoCastVictim(SPELL_PHOENIX_FIREBALL);
                _fireballTimer = TIMER_PHEONIX_FIREBALL;
            }
            else
                DoCastVictim(SPELL_PHOENIX_FIREBALL);
        }

        bool CanAIAttack(Unit const* who) const override
        {
            Unit* owner = me->GetOwner();
            return owner && who->IsAlive() && me->IsValidAttackTarget(who) &&
                !who->HasBreakableByDamageCrowdControlAura() &&
                who->IsInCombatWith(owner) && ScriptedAI::CanAIAttack(who);
        }

        // Do not reload Creature templates on evade mode enter - prevent visual lost
        void EnterEvadeMode(EvadeReason /*why*/) override
        {
            if (me->IsInEvadeMode() || !me->IsAlive())
                return;

            Unit* owner = me->GetCharmerOrOwner();

            me->CombatStop(true);
            if (owner && !me->HasUnitState(UNIT_STATE_FOLLOW))
            {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
            }
        }

        uint32 _fireballTimer = 0;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pet_phoenix_AI(creature);
    }
};

void AddSC_phoenix_scripts()
{
    new npc_pet_phoenix();
}
