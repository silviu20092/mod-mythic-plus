/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "GameTime.h"
#include "mythic_plus.h"

class mythic_plus_playerscript : public PlayerScript
{
public:
    mythic_plus_playerscript() : PlayerScript("mythic_plus_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_PLAYER_JUST_DIED
        }
    )
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!sMythicPlus->IsEnabled())
            return;

        // check if the saved M+ level is still present (maybe it was removed from DB in the meantime)
        uint32 playerMplusLevel = sMythicPlus->GetCurrentMythicPlusLevel(player);
        if (playerMplusLevel > 0 && !sMythicPlus->GetMythicLevel(playerMplusLevel))
            sMythicPlus->SetCurrentMythicPlusLevel(player, 0, true); // force reset the level, this is an edge case anyway

        Group* group = player->GetGroup();
        if (group != nullptr)
        {
            ObjectGuid leaderGuid = group->GetLeaderGUID();
            uint32 mplusLevel = sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
            if (mplusLevel > 0)
                MythicPlus::BroadcastToPlayer(player, "Your group's leader (can even be you) has a Mythic Plus level set. The leader can use a Mythic Keystone to start a level " + Acore::ToString(mplusLevel) + " Mythic Plus dungeon!");
        }
    }

    void OnPlayerJustDied(Player* player) override
    {
        if (player && sMythicPlus->IsInMythicPlus(player))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(player->GetMap(), false);
            ASSERT(mapData);

            if (mapData->penaltyOnDeath > 0 && !mapData->done)
            {
                std::ostringstream oss;
                oss << player->GetName() << " just died, a penalty of ";
                oss << secsToTimeString(mapData->penaltyOnDeath);
                oss << " was applied.";

                Map* map = player->GetMap();
                sMythicPlus->BroadcastToMap(player->GetMap(), MythicPlus::Utils::RedColored(oss.str()));

                mapData->deaths++;
                
                sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), mapData->timeLimit, mapData->mythicPlusStartTimer, mapData->mythicLevel->level, mapData->penaltyOnDeath, mapData->deaths, mapData->done);
            }
        }
    }
};

void AddSC_mythic_plus_playerscript()
{
    new mythic_plus_playerscript();
}
