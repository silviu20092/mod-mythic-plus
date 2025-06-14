/*
 * Credits: silviu20092
 */

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

        if (roll_chance_i(40))
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
