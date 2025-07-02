/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "mythic_plus.h"

class mythic_plus_keystone : public ItemScript
{
public:
    mythic_plus_keystone() : ItemScript("mythic_plus_keystone") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        Map* map = player->GetMap();
        if (sMythicPlus->CanMapBeMythicPlus(map))
        {
            // only group's leader can use the keystone
            if (!MythicPlus::Utils::IsGroupLeader(player))
                MythicPlus::BroadcastToPlayer(player, "Only the group's leader can use the keystone.");
            else
            {
                // now check if the group's leader actually has a set m+ level
                if (sMythicPlus->GetCurrentMythicPlusLevel(player) == 0)
                    MythicPlus::BroadcastToPlayer(player, "You don't have a Mythic Plus level chosen.");
                else
                {
                    MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map);
                    if (mapData->mythicPlusStartTimer > 0)
                        MythicPlus::BroadcastToPlayer(player, "Mythic Plus dungeon already in progress or completed.");
                    else if (mapData->keystoneTimer > 0)
                        MythicPlus::BroadcastToPlayer(player, "Keystone was already used, waiting for Mythic Plus to start...");
                    else
                    {
                        // lets check if a dungeon save was performed, in which case it must be saved as M+
                        const MythicPlus::MythicPlusDungeonInfo* dsave = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());
                        if (dsave != nullptr && !dsave->isMythic)
                            MythicPlus::BroadcastToPlayer(player, "This dungeon was marked as non Mythic Plus, keystone can't be used anymore.");
                        else
                        {
                            if (player->IsInCombat())
                                MythicPlus::BroadcastToPlayer(player, "Can't use the Keystone while in combat.");
                            else
                            {
                                // every player in the group must be online and at max level
                                if (!sMythicPlus->CheckGroupLevelForKeystone(player))
                                    MythicPlus::BroadcastToPlayer(player, "All players in the group must be online and at max level.");
                                else
                                {
                                    mapData->keystoneTimer = MythicPlus::Utils::GameTimeCount();
                                    mapData->keystoneLevel = sMythicPlus->GetCurrentMythicPlusLevel(player);
                                    std::ostringstream oss;
                                    oss << "Mythic Plus will start in ";
                                    oss << secsToTimeString(MythicPlus::KEYSTONE_START_TIMER / 1000);
                                    oss << ". Mythic level set: ";
                                    oss << Acore::ToString(mapData->keystoneLevel);
                                    MythicPlus::AnnounceToGroup(player, oss.str());

                                    sMythicPlus->RemoveKeystone(player);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
            MythicPlus::BroadcastToPlayer(player, "Need to be in a Mythic Plus capable dungeon in order to use the keystone.");
        return true;
    }
};

void AddSC_mythic_plus_keystone()
{
    new mythic_plus_keystone();
}
