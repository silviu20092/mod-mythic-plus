/*
 * Credits: silviu20092
 */

#ifndef _MYTHIC_AFFIX_H_
#define _MYTHIC_AFFIX_H_

class Creature;
class TempSummon;

enum MythicAffixType
{
    AFFIX_TYPE_HEALTH_INCREASE,
    AFFIX_TYPE_HEALTH_INCREASE_TRASH,
    AFFIX_TYPE_HEALTH_INCREASE_BOSSES,
    AFFIX_TYPE_MULTIPLE_ENEMIES,
    AFFIX_TYPE_MORE_CREATURE_DAMAGE,
    AFFIX_TYPE_RANDOMLY_EXPLODE,
    MAX_AFFIX_TYPE
};

class MythicAffix
{
public:
    virtual MythicAffixType GetAffixType() const = 0;
    virtual std::string ToString() const = 0;

    virtual void HandleStaticEffect(Creature* creature) {}
    virtual void HandleOnDamageEffect(Unit* attacker, Unit* victim, uint32& damage) {}
    virtual void HandlePeriodicEffect(Unit* unit, uint32 diff) {}

    static MythicAffix* AffixFactory(MythicAffixType type, float val);
protected:
    static bool IsCreatureProcessed(Creature* creature);
};

class HealthIncreaseAffix : public MythicAffix
{
private:
    float healthMod;
    bool CanApplyHealthIncrease(Creature* creature) const;
public:
    HealthIncreaseAffix(float healthMod) : healthMod(healthMod) {}

    virtual MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_HEALTH_INCREASE;
    }

    std::string ToString() const override;

    void HandleStaticEffect(Creature* creature) override;

    float GetHealthMod() const
    {
        return healthMod;
    }

    virtual bool GetApplyForTrash() const
    {
        return true;
    }

    virtual bool GetApplyForBosses() const
    {
        return true;
    }
};

class TrashHealthIncreaseAffix : public HealthIncreaseAffix
{
public:
    TrashHealthIncreaseAffix(float healthMod) : HealthIncreaseAffix(healthMod) {}

    MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_HEALTH_INCREASE_TRASH;
    }

    bool GetApplyForBosses() const override
    {
        return false;
    }
};

class BossHealthIncreaseAffix : public HealthIncreaseAffix
{
public:
    BossHealthIncreaseAffix(float healthMod) : HealthIncreaseAffix(healthMod) {}

    MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_HEALTH_INCREASE_BOSSES;
    }

    bool GetApplyForTrash() const override
    {
        return false;
    }
};

class MultipleEnemiesAffix : public MythicAffix
{
private:
    float chance;

    TempSummon* DoCreateCopy(Creature* creature);
public:
    MultipleEnemiesAffix(float chance) : chance(chance) {}

    MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_MULTIPLE_ENEMIES;
    }

    void HandleStaticEffect(Creature* creature) override;
    std::string ToString() const override;
};

class MoreDamageForCreaturesAffix : public MythicAffix
{
private:
    float perc;
public:
    MoreDamageForCreaturesAffix(float perc) : perc(perc) {}

    MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_MORE_CREATURE_DAMAGE;
    }

    void HandleOnDamageEffect(Unit* attacker, Unit* victim, uint32& damage) override;
    std::string ToString() const override;
};

class RandomlyExplodeAffix : public MythicAffix
{
private:
    static constexpr uint32 EXPLOSION_VISUAL = 71495;
    std::unordered_map<uint32, uint32> timerMap;
public:
    RandomlyExplodeAffix() {}

    MythicAffixType GetAffixType() const override
    {
        return AFFIX_TYPE_RANDOMLY_EXPLODE;
    }

    void HandlePeriodicEffect(Unit* unit, uint32 diff) override;
    std::string ToString() const override;
};

#endif
