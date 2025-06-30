/*
 * Credits: silviu20092
 */

#include "gossip_support.h"

class Player;

class MythicPlusNpcSupport : public GossipSupport
{
public:
    void AddMythicPlusLevels(Player* player, Creature* creature);
    void AddMythicPlusLevelInfo(Player* player, Creature* creature, uint32 mythicLevel);
    void AddMythicPlusDungeonList(Player* player, Creature* creature);
    void AddMythicPlusDungeonListForSnapshots(Player* player, Creature* creature, uint32 snapMythicLevel);
    void AddMythicPlusSnapshotAllRuns(Player* player, Creature* creature, uint32 mapEntry);
    void AddMythicPlusAllLevels(Player* player, Creature* creature);
    void AddMythicPlusDungeonSnapshotDetails(Player* player, Creature* creature, uint32 internalId);
    void AddRandomAfixes(Player* player, Creature* creature);
    void AddRandomAffixesForLevel(Player* player, Creature* creature, uint32 level);
    bool TakePagedDataAction(Player* player, Creature* creature, uint32 action) override;
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override;
private:
    static bool CompareIdentifierById(const Identifier* a, const Identifier* b);

    struct MythicPlusNpcPageInfo : public PagedDataCustomInfo
    {
        uint32 mythicLevel = 0;
        uint32 mapEntry = 0;
        uint32 snapMythicLevel = 0;
        uint32 internalId = 0;
        uint32 randomMythicLevel = 0;
    };
protected:
    uint32 _PageZeroSender(const PagedData& pagedData) const override;
    void AddMainMenu(Player* player, Creature* creature) override;
};
