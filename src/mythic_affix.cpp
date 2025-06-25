/*
 * Credits: silviu20092
 */

#include <iomanip>
#include "Creature.h"
#include "mythic_plus.h"
#include "mythic_affix.h"

/*static*/ bool MythicAffix::IsCreatureProcessed(Creature* creature)
{
    MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
    if (creatureData == nullptr)
        return false;

    return creatureData->processed;
}

/*static*/ MythicAffix* MythicAffix::AffixFactory(MythicAffixType type, float val1, float val2)
{
    switch (type)
    {
        case AFFIX_TYPE_HEALTH_INCREASE:
            return new HealthIncreaseAffix(val1);
        case AFFIX_TYPE_HEALTH_INCREASE_TRASH:
            return new TrashHealthIncreaseAffix(val1);
        case AFFIX_TYPE_HEALTH_INCREASE_BOSSES:
            return new BossHealthIncreaseAffix(val1);
        case AFFIX_TYPE_MULTIPLE_ENEMIES:
            return new MultipleEnemiesAffix(val1);
        case AFFIX_TYPE_MORE_CREATURE_DAMAGE:
            return new MoreDamageForCreaturesAffix(val1);
        case AFFIX_TYPE_RANDOMLY_EXPLODE:
            return new RandomlyExplodeAffix();
        case AFFIX_TYPE_LIGHTNING_SPHERE:
            return new LightningSphereAffix((uint32)val1, val2);
        default:
            return nullptr;
    }
}

bool HealthIncreaseAffix::CanApplyHealthIncrease(Creature* creature) const
{
    if (!IsCreatureProcessed(creature))
        return false;

    if (!GetApplyForTrash() && !creature->IsDungeonBoss())
        return false;

    if (!GetApplyForBosses() && creature->IsDungeonBoss())
        return false;

    return true;
}

void HealthIncreaseAffix::HandleStaticEffect(Creature* creature)
{
    if (!CanApplyHealthIncrease(creature))
        return;

    MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
    ASSERT(creatureData);

    uint32 newMaxHealth = creatureData->originalMaxHealth + (uint32)(healthMod * creatureData->originalMaxHealth);
    creature->SetCreateHealth(newMaxHealth);
    creature->SetMaxHealth(newMaxHealth);
    creature->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)newMaxHealth);
    creature->SetHealth(newMaxHealth * (creature->GetHealthPct() / 100));
    creature->ResetPlayerDamageReq();
}

std::string HealthIncreaseAffix::ToString() const
{
    std::ostringstream oss;
    oss << "Health increased by ";
    oss << std::fixed << std::setprecision(2) << healthMod * 100 << "% for ";
    if (GetApplyForTrash() && GetApplyForBosses())
        oss << "trash and bosses";
    else if (!GetApplyForTrash())
        oss << "bosses only";
    else if (!GetApplyForBosses())
        oss << "trash only";
    else
        oss << "unknown";

    return oss.str();
}

void MultipleEnemiesAffix::HandleStaticEffect(Creature* creature)
{
    if (!IsCreatureProcessed(creature))
        return;

    if (sMythicPlus->IsCreatureIgnoredForMultiplyAffix(creature->GetEntry()))
        return;

    if (creature->IsDungeonBoss() || sMythicPlus->IsFinalBoss(creature->GetEntry()))
        return;

    if (creature->isDead())
        return;

    MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
    ASSERT(creatureData);

    if (creatureData->copy)
        return;

    if (!roll_chance_f(chance))
        return;

    DoCreateCopy(creature);
}

TempSummon* MultipleEnemiesAffix::DoCreateCopy(Creature* creature)
{
    float x = creature->GetPositionX() + irand(-10, 10);
    float y = creature->GetPositionY() + irand(-10, 10);
    float z = creature->GetPositionZ();
    TempSummon* summon = creature->SummonCreature(creature->GetEntry(), x, y, z, 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
    ASSERT(summon);
    MythicPlus::CreatureData* summonData = sMythicPlus->GetCreatureData(summon->ToCreature());
    summonData->copy = true;
    return summon;
}

std::string MultipleEnemiesAffix::ToString() const
{
    std::ostringstream oss;
    oss << "Trash enemies can spawn a copy of each other";
    oss << " (";
    oss << MythicPlus::Utils::FormatFloat(chance) << "% chance per each creature)";
    return oss.str();
}

std::string MoreDamageForCreaturesAffix::ToString() const
{
    std::ostringstream oss;
    oss << "All enemies deal ";
    oss << MythicPlus::Utils::FormatFloat(perc) << "% more damage";
    return oss.str();
}

void MoreDamageForCreaturesAffix::HandleOnDamageEffect(Unit* attacker, Unit* victim, uint32& damage)
{
    ASSERT(attacker && victim);

    if (!attacker->ToCreature())
        return;

    Creature* creature = attacker->ToCreature();
    MythicPlus::CreatureData* creatureData = sMythicPlus->GetCreatureData(creature, false);
    if (!creatureData || !creatureData->processed)
        return;

    damage = damage + (uint32)(perc / 100 * damage);
}

void RandomlyExplodeAffix::HandlePeriodicEffect(Unit* unit, uint32 diff)
{
    ASSERT(unit);
    if (!unit->ToPlayer() || unit->isDead())
        return;

    Player* player = unit->ToPlayer();
    uint32& timer = timerMap[player->GetGUID().GetCounter()];

    timer += diff;

    // check every 15 seconds
    if (timer >= 15000)
    {
        timer = 0;

        MythicPlus::MapData* mapData = sMythicPlus->GetMapData(player->GetMap(), false);
        ASSERT(mapData);

        if (roll_chance_i(40) && !mapData->done)
        {
            player->CastSpell(player, EXPLOSION_VISUAL, true);
            uint32 damage = (uint32)(frand(15, 35) / 100 * player->GetMaxHealth());
            Unit::DealDamage(player, player, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        }
    }
}

std::string RandomlyExplodeAffix::ToString() const
{
    return "Random explosions deal damage to players";
}

void LightningSphereAffix::HandlePeriodicEffectMap(Map* map, uint32 diff)
{
    if (!sMythicPlus->IsMapInMythicPlus(map))
        return;

    if (spawnTimer >= spawnTimerEnd)
    {
        spawnTimer = 0;

        if (roll_chance_f(chanceOfSpawn))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            // don't try to spawn spheres if the dungeon is finished
            if (mapData->done)
                return;

            Map::PlayerList const& playerList = map->GetPlayers();
            if (playerList.IsEmpty())
                return;

            std::list<Player*> players;
            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                if (Player* player = i->GetSource())
                    if (!player->isDead())
                        players.push_back(player);

            if (!players.empty())
            {
                Player* player = Acore::Containers::SelectRandomContainerElement(players);
                player->SummonCreature(MythicPlus::NPC_LIGHTNING_SPHERE, *player, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60 * 1000);
            }
        }
    }
    else
        spawnTimer += diff;
}

std::string LightningSphereAffix::ToString() const
{
    std::ostringstream oss;
    oss << "Periodically summons lightning spheres that deal huge damage to players if not killed fast [";
    oss << MythicPlus::Utils::FormatFloat(chanceOfSpawn) << "% chance to spawn every ";
    oss << secsToTimeString(spawnTimerEnd / 1000);
    oss << "]";
    return oss.str();
}
