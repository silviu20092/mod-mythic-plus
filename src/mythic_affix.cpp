/*
 * Credits: silviu20092
 */

#include <iomanip>
#include "Creature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
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
        case AFFIX_TYPE_RANDOM_ENEMY_ENRAGE:
            return new EnemyEnrageAffix();
        case AFFIX_TYPE_RANDOM_ENTANGLING_ROOTS:
            return new EntanglingRootsAffix();
        default:
            return nullptr;
    }
}

/*static*/ MythicAffix* MythicAffix::AffixFactory(MythicAffixType type)
{
    return AffixFactory(type, 0.0f, 0.0f);
}

/*static*/ std::vector<MythicAffix*> MythicAffix::GenerateRandom(uint32 maxCount)
{
    std::vector<MythicAffix*> res;
    if (maxCount > RANDOM_AFFIX_MAX_COUNT)
        return res;

    std::vector<uint32> chosen;
    std::ranges::sample(RandomAffixes, std::back_inserter(chosen), maxCount, MythicPlus::Utils::RandomEngine());
    for (auto affixType : chosen)
    {
        MythicAffix* affix = AffixFactory((MythicAffixType)affixType);
        ASSERT(affix);
        res.push_back(affix);
    }

    return res;
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
            Unit::DealDamage(player, player, damage, nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE, nullptr, false, true);
            player->SendAttackStateUpdate(HITINFO_NO_ANIMATION, player, 1, SPELL_SCHOOL_MASK_FIRE, damage, 0, 0, VICTIMSTATE_HIT, 0);
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

void EnemyEnrageAffix::HandlePeriodicEffect(Unit* unit, uint32 diff)
{
    if (!unit)
        return;

    if (!sMythicPlus->IsInMythicPlus(unit))
        return;

    if (!unit->ToCreature())
        return;

    Creature* creature = unit->ToCreature();
    if (creature->GetEntry() == MythicPlus::NPC_LIGHTNING_SPHERE)
        return;

    if (!IsCreatureProcessed(creature))
        return;

    if (!creature->IsInCombat())
        return;

    if (!creature->GetVictim() || !creature->GetVictim()->ToPlayer())
        return;

    uint32& lastTimer = timerMap[creature->GetGUID().GetCounter()];
    if (lastTimer >= checkAtTimer)
    {
        if (roll_chance_f(chance))
        {
            if (!creature->HasAura(ENRAGE_SPELL_ID))
                creature->CastSpell(creature, ENRAGE_SPELL_ID, true);
        }
        lastTimer = 0;
    }
    else
        lastTimer += diff;
}

std::string EnemyEnrageAffix::ToString() const
{
    return "Enemies (including bosses) can randomly enrage while in combat";
}

void EntanglingRootsAffix::HandlePeriodicEffect(Unit* unit, uint32 diff)
{
    if (!unit)
        return;

    if (!sMythicPlus->IsInMythicPlus(unit))
        return;

    if (!unit->ToPlayer())
        return;

    if (unit->isDead())
        return;

    MythicPlus::MapData* mapData = sMythicPlus->GetMapData(unit->GetMap(), false);
    ASSERT(mapData != nullptr);

    if (mapData->done)
        return;

    Player* player = unit->ToPlayer();

    uint32& lastTimer = timerMap[MythicPlus::Utils::PlayerGUID(player)];
    if (lastTimer >= checkAtTimer)
    {
        if (roll_chance_f(chance))
        {
            if (!player->HasAura(ENTANGLING_ROOTS_SPELL_ID))
            {
                static float srange = 100.0f;
                std::list<Unit*> targets;
                Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(player, player, srange);
                Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(player, targets, u_check);
                Cell::VisitObjects(player, searcher, srange);

                if (!targets.empty())
                {
                    auto foundUnitItr = Acore::Containers::SelectRandomContainerElementIf(targets, [&](const Unit* unit) -> bool
                    {
                        return unit->IsAlive()
                            && unit->GetLevel() >= player->GetLevel() // skip level 1 stuff from instances for example
                            && unit->IsWithinLOSInMap(player)
                            && unit->IsValidAttackTarget(player);
                    });
                    if (foundUnitItr != targets.end())
                        (*foundUnitItr)->CastSpell(player, ENTANGLING_ROOTS_SPELL_ID, true);
                }
            }
        }
        lastTimer = 0;
    }
    else
        lastTimer += diff;
}

std::string EntanglingRootsAffix::ToString() const
{
    return "Nearby enemies can cast entangling roots on players, freezing them in place and dealing damage";
}
