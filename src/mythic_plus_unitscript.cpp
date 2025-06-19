/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "GameTime.h"
#include "mythic_plus.h"
#include "mythic_affix.h"

class mythic_plus_unitscript : public UnitScript
{
public:
    mythic_plus_unitscript() : UnitScript("mythic_plus_unitscript", true,
        {
            UNITHOOK_ON_DAMAGE,
            UNITHOOK_ON_UNIT_DEATH,
            UNITHOOK_ON_UNIT_ENTER_EVADE_MODE
        })
    {
    }

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (attacker && victim && attacker->GetMap() == victim->GetMap())
        {
            Creature* creature = victim->ToCreature();
            if (!creature || (!creature->IsDungeonBoss() && !sMythicPlus->IsFinalBoss(creature->GetEntry())))
                return;

            Map* map = victim->GetMap();
            if (!sMythicPlus->IsMapInMythicPlus(map))
                return;

            MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature);
            if (creatureData->engageTimer > 0)
                return;

            if (!attacker->ToPlayer() && !attacker->IsControlledByPlayer())
                return;

            uint32 gameTime = GameTime::GetGameTime().count();
            creatureData->engageTimer = gameTime;

            // a boss was engaged so in 99.9% cases the mythic plus timer was already started, but double check anyway
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);
            if (mapData->mythicPlusStartTimer == 0)
            {
                mapData->mythicPlusStartTimer = gameTime;
                sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), mapData->timeLimit, mapData->mythicPlusStartTimer, mapData->mythicLevel->level, mapData->penaltyOnDeath, mapData->deaths, mapData->done);
            }

            const Map::PlayerList& playerList = map->GetPlayers();
            for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                if (Player* player = itr->GetSource())
                {
                    const std::string& cname = MythicPlus::Utils::GetCreatureName(player, creature);
                    std::ostringstream oss;
                    oss << "Engaged " << cname << ". Good luck!";
                    MythicPlus::AnnounceToPlayer(player, oss.str());
                    MythicPlus::BroadcastToPlayer(player, oss.str());
                }
            }
        }
    }

    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        if (unit && killer && unit->GetMap() == killer->GetMap())
        {
            Creature* creature = unit->ToCreature();
            if (!creature)
                return;

            MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
            if (creatureData == nullptr || creatureData->engageTimer == 0)
                return;

            Map* map = creature->GetMap();

            uint64 gameTime = GameTime::GetGameTime().count();
            uint64 diff = gameTime - creatureData->engageTimer;
            std::string downAfterStr = secsToTimeString(diff);

            MythicPlus::MythicPlusDungeonInfo* savedDungeon = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());
            ASSERT(savedDungeon);

            creatureData->engageTimer = 0;

            const Map::PlayerList& playerList = map->GetPlayers();
            for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                if (Player* player = itr->GetSource())
                {
                    const std::string& cname = MythicPlus::Utils::GetCreatureName(player, creature);
                    std::ostringstream oss;
                    oss << cname << " was bested in " << downAfterStr;
                    oss << ". Congratulations!";
                    MythicPlus::AnnounceToPlayer(player, oss.str());
                    MythicPlus::BroadcastToPlayer(player, oss.str());

                    bool finalBoss = sMythicPlus->IsFinalBoss(creature->GetEntry());
                    bool rewarded = false;
                    MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
                    ASSERT(mapData);

                    if (finalBoss)
                    {
                        std::ostringstream oss2;
                        oss2 << "Mythic Plus dungeon ended after ";
                        oss2 << secsToTimeString(gameTime - savedDungeon->startTime);
                        MythicPlus::AnnounceToPlayer(player, oss2.str());
                        MythicPlus::BroadcastToPlayer(player, oss2.str());

                        if (mapData->receiveLoot)
                        {
                            rewarded = true;
                            sMythicPlus->Reward(player, mapData->mythicLevel->reward);
                        }
                        mapData->done = true;

                        sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), mapData->timeLimit, mapData->mythicPlusStartTimer, mapData->mythicLevel->level, mapData->penaltyOnDeath, mapData->deaths, true);
                    }

                    sMythicPlus->AddDungeonSnapshot(map->GetInstanceId(),
                        map->GetId(),
                        map->GetDifficulty(),
                        savedDungeon->startTime,
                        gameTime,
                        diff,
                        savedDungeon->timeLimit,
                        player->GetGUID().GetCounter(),
                        player->GetName(),
                        savedDungeon->mythicLevel,
                        creature->GetEntry(),
                        finalBoss,
                        rewarded,
                        mapData->penaltyOnDeath,
                        mapData->deaths
                    );
                }
            }
        }
    }

    void OnUnitEnterEvadeMode(Unit* unit, uint8 /*evadeReason*/) override
    {
        if (unit && unit->ToCreature())
        {
            Creature* creature = unit->ToCreature();
            MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
            if (creatureData == nullptr)
                return;

            creatureData->engageTimer = 0;
        }
    }
};

class mythic_plus_unitscript_start_timer : public UnitScript
{
public:
    mythic_plus_unitscript_start_timer() : UnitScript("mythic_plus_unitscript_start_timer", true,
        {
            UNITHOOK_ON_UNIT_DEATH,
        })
    {
    }

    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        if (unit && killer && unit->GetMap() == killer->GetMap())
        {
            if (!sMythicPlus->IsInMythicPlus(unit))
                return;

            Creature* creature = unit->ToCreature();
            if (!creature || !sMythicPlus->CanProcessCreature(creature))
                return;

            // prevent timer start in cases like Drak'Tharon Keep where dungeon mobs kill each other at the start
            if (killer->ToCreature() != nullptr && !killer->IsControlledByPlayer())
                return;

            Map* map = unit->GetMap();
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);
            if (mapData->mythicPlusStartTimer == 0)
            {
                mapData->mythicPlusStartTimer = GameTime::GetGameTime().count();
                std::ostringstream oss;
                oss << "Mythic Plus timer started! ";
                oss << "Beat this timer to get loot: " << secsToTimeString(mapData->timeLimit) << ". ";
                oss << "Have fun!";
                MythicPlus::AnnounceToMap(map, oss.str());

                sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), mapData->timeLimit, mapData->mythicPlusStartTimer, mapData->mythicLevel->level, mapData->penaltyOnDeath, mapData->deaths, mapData->done);

                if (mapData->penaltyOnDeath > 0)
                    sMythicPlus->BroadcastToMap(map, MythicPlus::Utils::RedColored("Dying will give a penalty of " + secsToTimeString(mapData->penaltyOnDeath)));
            }
        }
    }
};

class mythic_plus_damage_affix_unitscript : public UnitScript
{
private:
    void HandleDamageEffect(Unit* attacker, Unit* victim, uint32& damage)
    {
        if (attacker && victim && attacker->GetMap() == victim->GetMap())
        {
            if (!sMythicPlus->IsMapInMythicPlus(attacker->GetMap()))
                return;

            Map* map = attacker->GetMap();
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);

            for (auto* affix : mapData->mythicLevel->affixes)
                affix->HandleOnDamageEffect(attacker, victim, damage);
        }
    }
public:
    mythic_plus_damage_affix_unitscript() : UnitScript("mythic_plus_damage_affix_unitscript", true,
        {
            UNITHOOK_MODIFY_MELEE_DAMAGE,
            UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
            UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK
        })
    {
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        HandleDamageEffect(attacker, target, damage);
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
    {
        // don't do anything for healing
        if (spellInfo->IsPositive() || damage <= 0)
            return;

        uint32 adjustedDamage = (uint32)damage;
        HandleDamageEffect(attacker, target, adjustedDamage);
        damage = (int32)adjustedDamage;
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
    {
        // don't do anything for healing
        if (spellInfo->IsPositive())
            return;

        HandleDamageEffect(attacker, target, damage);
    }
};

class mythic_plus_periodic_affix_unitscript : public UnitScript
{
public:
    mythic_plus_periodic_affix_unitscript() : UnitScript("mythic_plus_periodic_affix_unitscript", true,
        {
            UNITHOOK_ON_UNIT_UPDATE
        })
    {
    }

    void OnUnitUpdate(Unit* unit, uint32 diff) override
    {
        if (unit && sMythicPlus->IsInMythicPlus(unit))
        {
            Map* map = unit->GetMap();
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);

            for (auto* affix : mapData->mythicLevel->affixes)
                affix->HandlePeriodicEffect(unit, diff);
        }
    }
};

void AddSC_mythic_plus_unitscript()
{
    new mythic_plus_unitscript();
    new mythic_plus_unitscript_start_timer();
    new mythic_plus_damage_affix_unitscript();
    new mythic_plus_periodic_affix_unitscript();
}
