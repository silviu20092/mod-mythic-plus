/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "GameTime.h"
#include "mythic_affix.h"
#include "mythic_plus.h"

class mythic_plus_all_mapscript : public AllMapScript
{
public:
    mythic_plus_all_mapscript() : AllMapScript("mythic_plus_all_mapscript",
        {
            ALLMAPHOOK_ON_PLAYER_ENTER_ALL,
            ALLMAPHOOK_ON_MAP_UPDATE,
            ALLMAPHOOK_ON_DESTROY_INSTANCE
        })
    {
    }

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        // edge case when players were in a M+ dungeon, server was restarted and the system was disabled in the meantime
        if (!sMythicPlus->IsEnabled() && map->GetInstanceId() != 0)
        {
            const MythicPlus::MythicPlusDungeonInfo* dsave = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());
            if (dsave != nullptr)
            {
                if (dsave->mythicLevel > 0)
                {
                    MythicPlus::BroadcastToPlayer(player, "Tried to join a saved Mythic Plus instance but now the system is disabled.");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }
            }
            else
            {
                // save even non Mythic Plus dungeon in DB, this is required for further edge checks
                if (sMythicPlus->MatchMythicPlusMapDiff(map))
                    sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), 0, 0L, 0, false, false);
            }
        }

        if (sMythicPlus->CanMapBeMythicPlus(map))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map);
            const MythicLevel* mythicLevel = nullptr;
            uint32 instanceId = map->GetInstanceId();
            const MythicPlus::MythicPlusDungeonInfo* savedDungeon = sMythicPlus->GetSavedDungeonInfo(instanceId);
            if (savedDungeon == nullptr)
            {
                uint32 dungeonMythicLevel = sMythicPlus->GetCurrentMythicPlusLevelForDungeon(player);
                mythicLevel = sMythicPlus->GetMythicLevel(dungeonMythicLevel);
                if (dungeonMythicLevel > 0 && mythicLevel == nullptr)
                {
                    MythicPlus::BroadcastToPlayer(player, "Trying to enter a Mythic Plus dungeon with a non existant level");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }
                uint32 timeLimit = 0;
                uint32 mlevel = 0;
                if (mythicLevel != nullptr)
                {
                    timeLimit = mythicLevel->timeLimit;
                    mlevel = mythicLevel->level;
                }
                sMythicPlus->SaveDungeonInfo(instanceId, map->GetId(), timeLimit, 0L, mlevel, false);

                mapData->timeLimit = timeLimit;

                if (mythicLevel == nullptr)
                    MythicPlus::BroadcastToPlayer(player, "You entered a Mythic Plus capable dungeon but there is no Mythic Plus level set (you are either not in a group or the leader does not have any level set). This "
                        "dungeon is now saved as such (no Mythic Plus), you will need to reset the dungeon in order to join Mythic Plus");
            }
            else
            {
                if (!savedDungeon->isMythic)
                {
                    // this means that the instance was non Mythic Plus, but now the system was enabled and it could be M+
                    MythicPlus::BroadcastToPlayer(player, "This instance was saved as NON Mythic Plus but now the system was enabled.");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }

                mythicLevel = sMythicPlus->GetMythicLevel(savedDungeon->mythicLevel);
                if (savedDungeon->mythicLevel > 0 && mythicLevel == nullptr)
                {
                    // edge case where a dungeon was saved as M+ but now the level does not longer exist (removed from DB)
                    MythicPlus::BroadcastToPlayer(player, "This dungeon was saved as Mythic Plus but now it's level does no longer exist");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }

                mapData->mythicPlusStartTimer = savedDungeon->startTime;
                mapData->done = savedDungeon->done;
                mapData->timeLimit = savedDungeon->timeLimit;
            }

            // check if player can actually join Mythic Plus
            MythicPlus::MythicPlusDungeonEnterState enterState = sMythicPlus->CanEnterDungeon(map, player);
            if (enterState < MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_OK)
            {
                if (enterState == MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_NO_GROUP_LEADER)
                    MythicPlus::BroadcastToPlayer(player, "You must be in a group to join Mythic Plus --> (this is a saved Mythic Plus instance)");
                else if (enterState == MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_LEADER_OFFLINE)
                    MythicPlus::BroadcastToPlayer(player, "Group's leader must be online in order to join Mythic Plus --> (this is a saved Mythic Plus instance)");
                else if (enterState == MythicPlus::MythicPlusDungeonEnterState::ENTER_STATE_INVALID_DIFFICULTY)
                    MythicPlus::BroadcastToPlayer(player, "You need heroic difficulty for this dungeon in order to join Mythic Plus --> (this is a saved Mythic Plus instance)");

                MythicPlus::FallbackTeleport(player);
                return;
            }

            if (mythicLevel != nullptr)
            {
                if (!mapData->mythicLevel)
                {
                    MythicPlus::BroadcastToPlayer(player, "You have just joined a Mythic Plus dungeon. All affixes have been set and saved for this specific instance.");
                    mapData->mythicLevel = mythicLevel;
                }
                else
                    MythicPlus::BroadcastToPlayer(player, "You have joined an in progress Mythic Plus dungeon. All affixes were set and are active for this specific instance.");

                sMythicPlus->PrintMythicLevelInfo(mapData->mythicLevel, player);

                if (mapData->mythicPlusStartTimer == 0)
                    MythicPlus::AnnounceToPlayer(player, "Mythic Plus dungeon joined! Killing the first creature will start the timer.");
                else
                {
                    std::ostringstream oss;
                    if (!mapData->done)
                    {
                        long long diff = GameTime::GetGameTime().count() - mapData->mythicPlusStartTimer;
                        mapData->updateTimer = diff * 1000;
                        if (diff <= mapData->timeLimit)
                        {
                            oss << "Mythic Plus dungeon is in progress. Current timer: ";
                            oss << secsToTimeString(diff);
                            oss << ". Beat this timer to get loot: ";
                            oss << secsToTimeString(mapData->timeLimit);
                        }
                        else
                        {
                            oss << "Mythic Plus dungeon is in progress but no loot will be received. ";
                            oss << "Limit timer: " << secsToTimeString(mapData->timeLimit);
                            oss << ". Current timer: " << secsToTimeString(diff);
                            mapData->receiveLoot = false;
                        }
                    }
                    else
                        oss << "Joined a Mythic Plus dungeon that is already done.";
                    MythicPlus::AnnounceToPlayer(player, oss.str());
                }
            }
        }
    }

    void OnMapUpdate(Map* map, uint32 diff) override
    {
        if (sMythicPlus->IsMapInMythicPlus(map))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);
            if (mapData->mythicPlusStartTimer > 0 && mapData->receiveLoot && !mapData->done)
            {
                if (mapData->updateTimer / 1000 > mapData->timeLimit)
                {
                    MythicPlus::AnnounceToMap(map, "Time's up! You will not receive any Mythic Plus loot anymore.");
                    mapData->receiveLoot = false;
                }

                mapData->updateTimer += diff;
            }

            for (auto* affix : mapData->mythicLevel->affixes)
                affix->HandlePeriodicEffectMap(map, diff);
        }
    }

    void OnDestroyInstance(MapInstanced* /*mapInstanced*/, Map* map) override
    {
        sMythicPlus->RemoveDungeonInfo(map->GetInstanceId());
    }
};

void AddSC_mythic_plus_all_mapscript()
{
    new mythic_plus_all_mapscript();
}
