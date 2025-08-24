/*
 * Credits: silviu20092
 */

#include "ScriptedCreature.h"
#include "CreatureScript.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "mythic_plus.h"

class npc_lightning_sphere : public CreatureScript
{
private:
    static constexpr uint32 LIGHTNING_VISUAL = 61585;
public:
    npc_lightning_sphere() : CreatureScript("npc_lightning_sphere") {}

    struct npc_lightning_sphereAI : public ScriptedAI
    {
    private:
        uint32 castTimer = 0;
        static constexpr float range = 100.0f;
    public:
        npc_lightning_sphereAI(Creature* creature) : ScriptedAI(creature) {}

        void UpdateAI(uint32 diff) override
        {
            if (castTimer >= 1200)
            {
                castTimer = 0;

                Map* map = me->GetMap();
                if (!sMythicPlus->IsMapInMythicPlus(map)) // this shouldn't really be there, but anyway ...
                    return;

                MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
                if (mapData->done)
                    return;

                std::list<Player*> playerList;
                Acore::AnyPlayerInObjectRangeCheck checker(me, range);
                Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, playerList, checker);
                Cell::VisitObjects(me, searcher, range);

                me->CastSpell(me, LIGHTNING_VISUAL, true);
                for (auto* p : playerList)
                {
                    uint32 damage = (uint32)(frand(1.5f, 5.25f) / 100 * p->GetMaxHealth());
                    Unit::DealDamage(me, p, damage, nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_SPELL, nullptr, false, true);
                    me->SendAttackStateUpdate(HITINFO_NO_ANIMATION, p, 1, SPELL_SCHOOL_MASK_SPELL, damage, 0, 0, VICTIMSTATE_HIT, 0);
                }
            }
            else
                castTimer += diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lightning_sphereAI(creature);
    }
};

void AddSC_npc_lightning_sphere()
{
    new npc_lightning_sphere();
}
