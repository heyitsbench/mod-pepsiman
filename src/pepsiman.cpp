#include "pepsiman_loader.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "TaskScheduler.h"

enum creatureIDs
{
    GOOD_BENCH     = 880000,
    BAD_BENCH      = 880001,
    COLA_CEO       = 880002,
    MINI_ELEMENTAL = 880003,
    DR_ELEMENTAL   = 880004
};

enum phaseEvents
{
    INTRO_EVENT           = 1,
    CAST_BENCH            = 2,
    MUSICAL_BENCH_START_1 = 3,
    MUSICAL_BENCHES_1     = 4,
    TRANSITION            = 5,
    SUPPORT_SPAWN         = 6,
    MUSICAL_BENCH_START_2 = 7,
    MUSICAL_BENCHES_2     = 8,
    ENRAGE                = 9
};

enum supportSpawns
{
    VENDOR_ENDERS = 1,
    PEPPER_ADDS   = 2,
    COCA_COLA     = 3,
    WORGOBLIN     = 4
};

enum benchGObjects
{
    BENCHED_GOBJECT = 1,
    NORMAL_BENCH    = 2,
    CURSED_BENCH    = 3
};

enum spells
{
    BENCH_STUN = 35856
};

enum talkMessages
{
    BENCH_ENGAGE = 0
};

class pepsiman : public CreatureScript {

public:
    pepsiman() : CreatureScript("pepsiman") { };

    struct pepsimanBoss : public ScriptedAI
    {
        pepsimanBoss(Creature* creature) : ScriptedAI(creature), summons(me) {}

        void EnterCombatSelfFunction()
        {
            Map::PlayerList const& PlList = me->GetMap()->GetPlayers();
            if (PlList.IsEmpty())
                return;

            for (const auto& i : PlList)
            {
                if (Player* player = i.GetSource())
                {
                    if (player->IsGameMaster())
                        continue;

                    if (player->IsAlive() && me->GetDistance(player) < 50.0f)
                    {
                        me->SetInCombatWith(player);
                        player->SetInCombatWith(me);
                        me->AddThreat(player, 0.0f);
                    }
                }
            }
        }

        void JustEngagedWith(Unit* who) override
        {
            ScriptedAI::JustEngagedWith(who);
            EnterCombatSelfFunction();
            events.ScheduleEvent(INTRO_EVENT, 2s, 0, 0);
            Talk(BENCH_ENGAGE);
        }

        void Reset() override
        {
            ScriptedAI::Reset();
            if (me->IsVisible())
            {
                me->SetReactState(REACT_AGGRESSIVE);
            }
            events.Reset();
            summons.DespawnAll();
            me->SendClearTarget();
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventID = events.ExecuteEvent())
            {
                switch (eventID)
                {
                case INTRO_EVENT:
                    //yell something
                    events.ScheduleEvent(CAST_BENCH, 10s, 0, 0);
                    events.ScheduleEvent(MUSICAL_BENCH_START_1, 2s, 0, 0);
                    return;
                case CAST_BENCH:
                    //yell mean words

                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50.0f, true))
                    {
                        //cast stun
                        me->SummonGameObject(BENCHED_GOBJECT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    }
                    events.Repeat(10s);
                    return;
                case MUSICAL_BENCH_START_1:
                    if (!me->HealthBelowPct(75))
                    {
                        return;
                    }
                    //yell something
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->SetImmuneToAll(true, true);

                    events.DelayEvents(15s);
                    events.ScheduleEvent(MUSICAL_BENCHES_1, 2s, 0, 0);
                    return;
                case MUSICAL_BENCHES_1:
                    //yell something
                    //spawn benches

                    events.ScheduleEvent(TRANSITION, 8s, 0, 0);
                    return;
                case TRANSITION:
                    //do a check to reduce player health, give buffs/debuffs

                    me->RemoveAllGameObjects();
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    return;
                case SUPPORT_SPAWN:
                {
                    if (!me->HealthBelowPct(50))
                    {
                        return;
                    }

                    uint8 supportSpawn = urand(1, 4);
                    switch (supportSpawn)
                    {
                    case VENDOR_ENDERS:
                        return;
                    case PEPPER_ADDS:
                        return;
                    case COCA_COLA:
                        return;
                    case WORGOBLIN:
                        return;
                    }
                    events.ScheduleEvent(MUSICAL_BENCH_START_2, 2s, 0, 0);
                    return;
                }
                case MUSICAL_BENCH_START_2:
                    if (!me->HealthBelowPct(30))
                    {
                        return;
                    }
                    //yell something

                    events.DelayEvents(15s);
                    events.ScheduleEvent(MUSICAL_BENCHES_2, 2s, 0, 0);
                    return;
                case MUSICAL_BENCHES_2:
                    //yell something
                    //spawn benches

                    events.ScheduleEvent(ENRAGE, 2s, 0, 0);
                    return;
                case ENRAGE:
                    if (!me->HealthBelowPct(10))
                    {
                        return;
                    }
                    events.CancelEvent(CAST_BENCH);
                    //yell mean words
                    
                    if (Unit* target = SelectTarget(SelectTargetMethod::MaxThreat, 0, 50.0f, true))
                    {
                        //cast stun
                        me->SummonGameObject(BENCHED_GOBJECT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    }
                    events.Repeat(5s);
                    return;
                }
            }
            if (!UpdateVictim())
            {
                return;
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        SummonList summons;
    };


    CreatureAI* GetAI(Creature* creature) const override
    {
        return new pepsimanBoss(creature);
    }
};

void Add_pepsiman()
{
    new pepsiman();
}
