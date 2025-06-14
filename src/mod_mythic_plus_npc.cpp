/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "mythic_plus.h"
#include "mythic_plus_npc_support.h"

class mod_mythic_plus_npc : public CreatureScript
{
private:
    MythicPlusNpcSupport* npcSupport;
public:
    mod_mythic_plus_npc() : CreatureScript("mod_mythic_plus_npc")
    {
        npcSupport = new MythicPlusNpcSupport();
    }

    ~mod_mythic_plus_npc()
    {
        delete npcSupport;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        return npcSupport->OnGossipHello(player, creature);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        return npcSupport->OnGossipSelect(player, creature, sender, action);
    }
};

void AddSC_mod_mythic_plus_npc()
{
    new mod_mythic_plus_npc();
}
