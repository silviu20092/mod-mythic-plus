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
            PLAYERHOOK_CAN_ENTER_MAP,
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_PLAYER_JUST_DIED
        }
    )
    {
    }

    bool OnPlayerCanEnterMap(Player* player, MapEntry const* entry, InstanceTemplate const* /*instance*/, MapDifficulty const* /*mapDiff*/, bool /*loginCheck*/) override
    {
        MythicPlus::MythicPlusDungeonEnterState enterState = sMythicPlus->CanEnterDungeonFromEntry(entry, player);
        if (enterState < MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_OK)
        {
            if (enterState == MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_LEADER_OFFLINE)
                MythicPlus::BroadcastToPlayer(player, "Group's leader must be online in order to join Mythic Plus --> (as you are in a group and the leader has a Mythic Plus level set this check is performed even if instance save data might be different)");

            return false;
        }

        return true;
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!sMythicPlus->IsEnabled())
            return;

        Group* group = player->GetGroup();
        if (group != nullptr)
        {
            ObjectGuid leaderGuid = group->GetLeaderGUID();
            uint32 mplusLevel = sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
            if (mplusLevel > 0)
                MythicPlus::BroadcastToPlayer(player, "Your group's leader (can even be you) has a Mythic Plus level set. You will automatically join Mythic Plus dungeons. Current set level: " + Acore::ToString(mplusLevel));
        }
    }

    void OnPlayerJustDied(Player* player) override
    {
        if (player && sMythicPlus->IsInMythicPlus(player))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(player->GetMap(), false);
            ASSERT(mapData);

            if (mapData->penaltyOnDeath > 0 && mapData->mythicPlusStartTimer > 0 && !mapData->done)
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
