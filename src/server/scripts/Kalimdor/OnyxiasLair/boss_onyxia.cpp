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

/* ScriptData
SDName: Boss_Onyxia
SD%Complete: 95
SDComment: <Known bugs>
               Ground visual for Deep Breath effect;
               Not summoning whelps on phase 3 (lacks info)
           </Known bugs>
SDCategory: Onyxia's Lair
EndScriptData */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "onyxias_lair.h"
#include "ScriptedCreature.h"
#include "TemporarySummon.h"

enum Yells
{
    // Say
    SAY_AGGRO                   = 0,
    SAY_KILL                    = 1,
    SAY_PHASE_2_TRANS           = 2,
    SAY_PHASE_3_TRANS           = 3,
    // Emote
    EMOTE_BREATH                = 4
};

enum Spells
{
    // Phase 1 spells
    SPELL_WING_BUFFET           = 18500,
    SPELL_FLAME_BREATH          = 18435,
    SPELL_CLEAVE                = 68868,
    SPELL_TAIL_SWEEP            = 68867,

    // Phase 2 spells
    SPELL_DEEP_BREATH           = 23461,
    SPELL_FIREBALL              = 18392,

    //Not much choise about these. We have to make own defintion on the direction/start-end point
    SPELL_BREATH_NORTH_TO_SOUTH = 17086,                    // 20x in "array"
    SPELL_BREATH_SOUTH_TO_NORTH = 18351,                    // 11x in "array"

    SPELL_BREATH_EAST_TO_WEST   = 18576,                    // 7x in "array"
    SPELL_BREATH_WEST_TO_EAST   = 18609,                    // 7x in "array"

    SPELL_BREATH_SE_TO_NW       = 18564,                    // 12x in "array"
    SPELL_BREATH_NW_TO_SE       = 18584,                    // 12x in "array"
    SPELL_BREATH_SW_TO_NE       = 18596,                    // 12x in "array"
    SPELL_BREATH_NE_TO_SW       = 18617,                    // 12x in "array"

    //SPELL_BREATH                = 21131,                  // 8x in "array", different initial cast than the other arrays

    // Phase 3 spells
    SPELL_BELLOWING_ROAR         = 18431
};

enum Events
{
    EVENT_BELLOWING_ROAR = 1,
    EVENT_FLAME_BREATH   = 2,
    EVENT_TAIL_SWEEP     = 3,
    EVENT_CLEAVE         = 4,
    EVENT_WING_BUFFET    = 5,
    EVENT_DEEP_BREATH    = 6,
    EVENT_MOVEMENT       = 7,
    EVENT_FIREBALL       = 8,
    EVENT_LAIR_GUARD     = 9,
    EVENT_WHELP_SPAWN    = 10
};

struct OnyxMove
{
    uint8 LocId;
    uint8 LocIdEnd;
    uint32 SpellId;
    float fX, fY, fZ;
};

static OnyxMove MoveData[8]=
{
    {0, 1, SPELL_BREATH_WEST_TO_EAST,   -33.5561f, -182.682f, -56.9457f}, //west
    {1, 0, SPELL_BREATH_EAST_TO_WEST,   -31.4963f, -250.123f, -55.1278f}, //east
    {2, 4, SPELL_BREATH_NW_TO_SE,         6.8951f, -180.246f, -55.896f}, //north-west
    {3, 5, SPELL_BREATH_NE_TO_SW,        10.2191f, -247.912f, -55.896f}, //north-east
    {4, 2, SPELL_BREATH_SE_TO_NW,       -63.5156f, -240.096f, -55.477f}, //south-east
    {5, 3, SPELL_BREATH_SW_TO_NE,       -58.2509f, -189.020f, -55.790f}, //south-west
    {6, 7, SPELL_BREATH_SOUTH_TO_NORTH, -65.8444f, -213.809f, -55.2985f}, //south
    {7, 6, SPELL_BREATH_NORTH_TO_SOUTH,  22.8763f, -217.152f, -55.0548f}, //north
};

Position const MiddleRoomLocation = {-23.6155f, -215.357f, -55.7344f, 0.0f};

Position const Phase2Location = {-80.924f, -214.299f, -82.942f, 0.0f};
Position const Phase2Floating = { -80.924f, -214.299f, -57.942f, 0.0f };

Position const SpawnLocations[3]=
{
    //Whelps
    {-30.127f, -254.463f, -89.440f, 0.0f},
    {-30.817f, -177.106f, -89.258f, 0.0f},
    //Lair Guard
    {-145.950f, -212.831f, -68.659f, 0.0f}
};

class boss_onyxia : public CreatureScript
{
public:
    boss_onyxia() : CreatureScript("boss_onyxia") { }

    struct boss_onyxiaAI : public BossAI
    {
        boss_onyxiaAI(Creature* creature) : BossAI(creature, DATA_ONYXIA)
        {
            Initialize();
        }

        void Initialize()
        {
            Phase = PHASE_START;
            MovePoint = urand(0, 5);
            PointData = GetMoveData();
            SummonWhelpCount = 0;
            triggerGUID.Clear();
            tankGUID.Clear();
            IsMoving = false;
        }

        void Reset() override
        {
            Initialize();

            if (!IsCombatMovementAllowed())
                SetCombatMovement(true);

            _Reset();
            me->SetReactState(REACT_AGGRESSIVE);
            instance->SetData(DATA_ONYXIA_PHASE, Phase);
            instance->DoStopCriteriaTimer(CriteriaStartEvent::SendEvent, ACHIEV_TIMED_START_EVENT);
        }

        void JustEngagedWith(Unit* who) override
        {
            BossAI::JustEngagedWith(who);
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_FLAME_BREATH, 10s, 20s);
            events.ScheduleEvent(EVENT_TAIL_SWEEP, 15s, 20s);
            events.ScheduleEvent(EVENT_CLEAVE, 2s, 5s);
            events.ScheduleEvent(EVENT_WING_BUFFET, 10s, 20s);
            instance->DoStartCriteriaTimer(CriteriaStartEvent::SendEvent, ACHIEV_TIMED_START_EVENT);
        }

        void JustSummoned(Creature* summoned) override
        {
            DoZoneInCombat(summoned);
            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                summoned->AI()->AttackStart(target);

            switch (summoned->GetEntry())
            {
                case NPC_WHELP:
                    ++SummonWhelpCount;
                    break;
                case NPC_LAIRGUARD:
                    summoned->setActive(true);
                    summoned->SetFarVisible(true);
                    break;
            }
            summons.Summon(summoned);
        }


        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_KILL);
        }

        void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_BREATH_EAST_TO_WEST ||
                spellInfo->Id == SPELL_BREATH_WEST_TO_EAST ||
                spellInfo->Id == SPELL_BREATH_SE_TO_NW ||
                spellInfo->Id == SPELL_BREATH_NW_TO_SE ||
                spellInfo->Id == SPELL_BREATH_SW_TO_NE ||
                spellInfo->Id == SPELL_BREATH_NE_TO_SW)
            {
                PointData = GetMoveData();
                MovePoint = PointData->LocIdEnd;

                me->SetSpeedRate(MOVE_FLIGHT, 1.5f);
                me->GetMotionMaster()->MovePoint(8, MiddleRoomLocation);
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case 8:
                        PointData = GetMoveData();
                        if (PointData)
                        {
                            me->SetSpeedRate(MOVE_FLIGHT, 1.0f);
                            me->GetMotionMaster()->MovePoint(PointData->LocId, PointData->fX, PointData->fY, PointData->fZ);
                        }
                        break;
                    case 9:
                        me->SetCanFly(false);
                        me->SetDisableGravity(false);
                        me->SetAnimTier(UNIT_BYTE1_FLAG_NONE, false);
                        if (Creature* trigger = ObjectAccessor::GetCreature(*me, triggerGUID))
                            Unit::Kill(me, trigger);
                        me->SetReactState(REACT_AGGRESSIVE);
                        // tank selection based on phase one. If tank is not there i take nearest one
                        if (Unit* tank = ObjectAccessor::GetUnit(*me, tankGUID))
                            me->GetMotionMaster()->MoveChase(tank);
                        else if (Unit* newtarget = SelectTarget(SelectTargetMethod::MinDistance, 0))
                            me->GetMotionMaster()->MoveChase(newtarget);
                        events.ScheduleEvent(EVENT_BELLOWING_ROAR, 5s);
                        events.ScheduleEvent(EVENT_FLAME_BREATH, 10s, 20s);
                        events.ScheduleEvent(EVENT_TAIL_SWEEP, 15s, 20s);
                        events.ScheduleEvent(EVENT_CLEAVE, 2s, 5s);
                        events.ScheduleEvent(EVENT_WING_BUFFET, 15s, 30s);
                        break;
                    case 10:
                        me->SetCanFly(true);
                        me->SetDisableGravity(true);
                        me->SetAnimTier(UnitBytes1_Flags(UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER), false);
                        me->SetFacingTo(me->GetOrientation() + float(M_PI));
                        if (Creature * trigger = me->SummonCreature(NPC_TRIGGER, MiddleRoomLocation, TEMPSUMMON_CORPSE_DESPAWN))
                            triggerGUID = trigger->GetGUID();
                        me->GetMotionMaster()->MoveTakeoff(11, Phase2Floating);
                        me->SetSpeedRate(MOVE_FLIGHT, 1.0f);
                        Talk(SAY_PHASE_2_TRANS);
                        instance->SetData(DATA_ONYXIA_PHASE, Phase);
                        events.ScheduleEvent(EVENT_WHELP_SPAWN, 5s);
                        events.ScheduleEvent(EVENT_LAIR_GUARD, 15000);
                        events.ScheduleEvent(EVENT_DEEP_BREATH, 75000);
                        events.ScheduleEvent(EVENT_MOVEMENT, 10s);
                        events.ScheduleEvent(EVENT_FIREBALL, 18s);
                        break;
                    case 11:
                        if (PointData)
                            me->GetMotionMaster()->MovePoint(PointData->LocId, PointData->fX, PointData->fY, PointData->fZ);
                        me->GetMotionMaster()->MoveIdle();
                        break;
                    default:
                        IsMoving = false;
                        break;
                }
            }
        }

        void SpellHitTarget(WorldObject* target, SpellInfo const* spellInfo) override
        {
            //Workaround - Couldn't find a way to group this spells (All Eruption)
            if (((spellInfo->Id >= 17086 && spellInfo->Id <= 17095) ||
                (spellInfo->Id == 17097) ||
                (spellInfo->Id >= 18351 && spellInfo->Id <= 18361) ||
                (spellInfo->Id >= 18564 && spellInfo->Id <= 18576) ||
                (spellInfo->Id >= 18578 && spellInfo->Id <= 18607) ||
                (spellInfo->Id == 18609) ||
                (spellInfo->Id >= 18611 && spellInfo->Id <= 18628) ||
                (spellInfo->Id >= 21132 && spellInfo->Id <= 21133) ||
                (spellInfo->Id >= 21135 && spellInfo->Id <= 21139) ||
                (spellInfo->Id >= 22191 && spellInfo->Id <= 22202) ||
                (spellInfo->Id >= 22267 && spellInfo->Id <= 22268)) &&
                (target->GetTypeId() == TYPEID_PLAYER))
            {
                instance->SetData(DATA_SHE_DEEP_BREATH_MORE, FAIL);
            }
        }

        OnyxMove* GetMoveData()
        {
            uint8 MaxCount = sizeof(MoveData) / sizeof(OnyxMove);

            for (uint8 i = 0; i < MaxCount; ++i)
            {
                if (MoveData[i].LocId == MovePoint)
                    return &MoveData[i];
            }

            return nullptr;
        }

        void SetNextRandomPoint()
        {
            uint8 MaxCount = sizeof(MoveData) / sizeof(OnyxMove);

            uint8 iTemp = urand(0, MaxCount - 1);

            if (iTemp >= MovePoint)
                ++iTemp;

            MovePoint = iTemp;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            //Common to PHASE_START && PHASE_END
            if (Phase == PHASE_START || Phase == PHASE_END)
            {
                //Specific to PHASE_START || PHASE_END
                if (Phase == PHASE_START)
                {
                    if (HealthBelowPct(65))
                    {
                        if (Unit* target = me->GetVictim())
                            tankGUID = target->GetGUID();
                        SetCombatMovement(false);
                        Phase = PHASE_BREATH;
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MovePoint(10, Phase2Location);
                        return;
                    }
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BELLOWING_ROAR: // Phase PHASE_END
                        {
                            DoCastVictim(SPELL_BELLOWING_ROAR);
                            // Eruption
                            GameObject* Floor = nullptr;
                            Trinity::GameObjectInRangeCheck check(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 15);
                            Trinity::GameObjectLastSearcher<Trinity::GameObjectInRangeCheck> searcher(me, Floor, check);
                            Cell::VisitGridObjects(me, searcher, 30.0f);
                            if (Floor)
                                instance->SetGuidData(DATA_FLOOR_ERUPTION_GUID, Floor->GetGUID());
                            events.ScheduleEvent(EVENT_BELLOWING_ROAR, 30s);
                            break;
                        }
                        case EVENT_FLAME_BREATH:   // Phase PHASE_START and PHASE_END
                            DoCastVictim(SPELL_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 10s, 20s);
                            break;
                        case EVENT_TAIL_SWEEP:     // Phase PHASE_START and PHASE_END
                            DoCastAOE(SPELL_TAIL_SWEEP);
                            events.ScheduleEvent(EVENT_TAIL_SWEEP, 15s, 20s);
                            break;
                        case EVENT_CLEAVE:         // Phase PHASE_START and PHASE_END
                            DoCastVictim(SPELL_CLEAVE);
                            events.ScheduleEvent(EVENT_CLEAVE, 2s, 5s);
                            break;
                        case EVENT_WING_BUFFET:    // Phase PHASE_START and PHASE_END
                            DoCastVictim(SPELL_WING_BUFFET);
                            events.ScheduleEvent(EVENT_WING_BUFFET, 15s, 30s);
                            break;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }
                DoMeleeAttackIfReady();
            }
            else
            {
                if (HealthBelowPct(40))
                {
                    Phase = PHASE_END;
                    instance->SetData(DATA_ONYXIA_PHASE, PHASE_END);
                    Talk(SAY_PHASE_3_TRANS);
                    SetCombatMovement(true);
                    IsMoving = false;
                    Position const pos = me->GetHomePosition();
                    me->GetMotionMaster()->MovePoint(9, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 12.0f);
                    events.ScheduleEvent(EVENT_BELLOWING_ROAR, 30s);
                    return;
                }

                if (!me->isMoving())
                    if (Creature* trigger = ObjectAccessor::GetCreature(*me, triggerGUID))
                        me->SetFacingToObject(trigger);

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEEP_BREATH:      // Phase PHASE_BREATH
                            if (!IsMoving)
                            {
                                if (me->IsNonMeleeSpellCast(false))
                                    me->InterruptNonMeleeSpells(false);

                                Talk(EMOTE_BREATH);
                                if (PointData) /// @todo: In what cases is this null? What should we do?
                                    DoCast(me, PointData->SpellId);
                                events.ScheduleEvent(EVENT_DEEP_BREATH, 75000);
                            }
                            else
                                events.ScheduleEvent(EVENT_DEEP_BREATH, 1s);
                            break;
                        case EVENT_MOVEMENT:         // Phase PHASE_BREATH
                            if (!IsMoving && !(me->HasUnitState(UNIT_STATE_CASTING)))
                            {
                                SetNextRandomPoint();
                                PointData = GetMoveData();

                                if (!PointData)
                                    return;

                                me->GetMotionMaster()->MovePoint(PointData->LocId, PointData->fX, PointData->fY, PointData->fZ);
                                IsMoving = true;
                                events.ScheduleEvent(EVENT_MOVEMENT, 25000);
                            }
                            else
                                events.ScheduleEvent(EVENT_MOVEMENT, 500ms);
                            break;
                        case EVENT_FIREBALL:         // Phase PHASE_BREATH
                            if (!IsMoving)
                            {
                                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                                    DoCast(target, SPELL_FIREBALL);
                                events.ScheduleEvent(EVENT_FIREBALL, 8s);
                            }
                            else
                                events.ScheduleEvent(EVENT_FIREBALL, 1s);
                            break;
                        case EVENT_LAIR_GUARD:       // Phase PHASE_BREATH
                            me->SummonCreature(NPC_LAIRGUARD, SpawnLocations[2], TEMPSUMMON_CORPSE_DESPAWN);
                            events.ScheduleEvent(EVENT_LAIR_GUARD, 30s);
                            break;
                        case EVENT_WHELP_SPAWN:      // Phase PHASE_BREATH
                            me->SummonCreature(NPC_WHELP, SpawnLocations[0], TEMPSUMMON_CORPSE_DESPAWN);
                            me->SummonCreature(NPC_WHELP, SpawnLocations[1], TEMPSUMMON_CORPSE_DESPAWN);
                            if (SummonWhelpCount >= RAID_MODE(20, 40))
                            {
                                SummonWhelpCount = 0;
                                events.ScheduleEvent(EVENT_WHELP_SPAWN, 90s);
                            }
                            else
                                events.ScheduleEvent(EVENT_WHELP_SPAWN, 500ms);
                            break;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }
            }
        }

        private:
            OnyxMove* PointData;
            uint8 Phase;
            uint8 MovePoint;
            uint8 SummonWhelpCount;
            ObjectGuid triggerGUID;
            ObjectGuid tankGUID;
            bool IsMoving;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetOnyxiaAI<boss_onyxiaAI>(creature);
    }
};

void AddSC_boss_onyxia()
{
    new boss_onyxia();
}
