#include "pepsiman_loader.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "TaskScheduler.h"

enum creatureIDs
{
    GOOD_BENCH = 880000,
    BAD_BENCH  = 880001
};

enum phaseEvents
{
    INTRO_EVENT         = 1,
    MUSICAL_BENCH_START = 2,
    MUSICAL_BENCHES     = 3,
    SUPPORT_SPAWN       = 4
};

enum supportSpawns
{
    VENDOR_ENDERS = 1,
    PEPPER_ADDS   = 2,
    COCA_COLA     = 3
};

enum spells
{
    BENCH_STUN = 35856
};

class pepsiman : public CreatureScript {

public:
    pepsiman() : CreatureScript("pepsiman") { };

    struct pepsimanBoss : public ScriptedAI
    {
        pepsimanBoss(Creature* creature) : ScriptedAI(creature), summons(me) {}

        void JustEngagedWith(Unit* who) override
        {
            ScriptedAI::JustEngagedWith(who);
            events.ScheduleEvent(INTRO_EVENT, 2s, 0, 0);
        }

        void Reset() override
        {
            ScriptedAI::Reset();
            if (me->IsVisible())
            {
                me->SetReactState(REACT_AGGRESSIVE);
            }
            events.Reset();
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
                    events.ScheduleEvent(MUSICAL_BENCH_START, 2s, 0, 0);
                    break;
                case MUSICAL_BENCH_START:
                    if (!me->HealthBelowPct(75))
                    {
                        return;
                    }
                    //yell something
                    //cast immunity spell or something (setimmunetoall)
                    events.ScheduleEvent(MUSICAL_BENCHES, 2s, 0, 0);
                    break;
                case MUSICAL_BENCHES:
                    //yell something
                    //spawn benches

                    break;
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
