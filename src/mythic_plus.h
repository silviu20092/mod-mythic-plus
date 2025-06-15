/*
 * Credits: silviu20092
 */

#ifndef _MYTHIC_PLUS_H_
#define _MYTHIC_PLUS_H_

#include "Player.h"

class MythicAffix;

struct MythicReward
{
    uint32 money = 0;
    std::vector<std::pair<uint32, uint32>> tokens;

    void AddToken(uint32 entry, uint32 count);
};

struct MythicLevel
{
    uint32 level;
    std::vector<MythicAffix*> affixes;
    uint32 timeLimit;
    MythicReward reward;
};
typedef std::vector<MythicLevel> MythicLevelContainer;

class MythicPlus
{
private:
    MythicPlus();
    ~MythicPlus();
public:
    static constexpr uint32 MYTHIC_SNAPSHOTS_TIMER_FREQ = 60 * 10 * 1000;

    class MapData : public DataMap::Base
    {
    public:
        MapData() {}

        const MythicLevel* mythicLevel = nullptr;
        long long mythicPlusStartTimer = 0;
        uint32 updateTimer = 0;
        bool receiveLoot = true;
        bool done = false;
    };

    class CreatureData : public DataMap::Base
    {
    public:
        CreatureData() {}

        uint32 originalCreateHealth = 0;
        uint32 originalMaxHealth = 0;
        bool processed = false;
        uint64 engageTimer = 0;
        bool copy = false;
    };

    enum MythicPlusDungeonEnterState
    {
        ENTER_STATE_NO_GROUP_LEADER,
        ENTER_STATE_LEADER_OFFLINE,
        ENTER_STATE_INVALID_DIFFICULTY,
        ENTER_STATE_OK
    };

    struct MythicPlusDungeonInfo
    {
        uint32 instanceId;
        uint32 mapId;
        uint64 startTime;
        uint32 mythicLevel;
        bool done;
        bool isMythic; // we save dungeon data for ALL dungeons, no matter if mythic or not
    };

    struct MythicPlusDungeonSnapshot
    {
        uint32 id;
        uint32 mapId;
        uint64 startTime;
        uint64 snapTime;
        uint32 combatTime;
        std::string players;
        uint32 mythicLevel;
        uint32 entry;
        bool finalBoss;
        bool rewarded;
        uint32 totalTime;
        uint32 internalId;
        uint32 endTime;
        uint32 difficulty;
    };

    class Utils
    {
    private:
        static constexpr int VISUAL_FEEDBACK_SPELL_ID = 46331;
    public:
        static std::string GetCreatureName(const Player* player, const Creature* creature);
        static std::string GetCreatureNameByEntry(const Player* player, uint32 entry);
        static uint32 PlayerGUID(const Player* player);
        static std::string CopperToMoneyStr(uint32 money, bool colored);
        static std::string ItemIcon(const ItemTemplate* proto, uint32 width, uint32 height, int x, int y);
        static std::string ItemIcon(const ItemTemplate* proto);
        static std::string ItemName(const ItemTemplate* proto, const Player* player);
        static std::string ItemLinkForUI(uint32 entry, const Player* player);
        static std::string Colored(const std::string& message, const std::string& hexColor);
        static std::string RedColored(const std::string& message);
        static std::string GreenColored(const std::string& message);
        static void VisualFeedback(Player* player);
        static std::string FormatFloat(float val, uint32 decimals = 2);
        static std::string DateFromSeconds(uint64 seconds);
    };

    struct DBAffix
    {
        uint32 level;
        uint16 affixType;
        float val1;
    };

    struct DBReward
    {
        enum RewardType
        {
            REWARD_COPPER,
            REWARD_TOKEN,
            MAX_REWARD_TYPE
        };
        uint32 level;
        RewardType type;    // 0 - copper (money), 1 - token (item)
        uint32 val1;        // for type = 0 - amount of copper, for type = 1 - the item entry
        uint32 val2;        // for type = 0 - nothing, for type = 1 - item count
    };
public:
    static MythicPlus* instance();

    MapData* GetMapData(Map* map, bool withDefault = true) const;
    CreatureData* GetCreatureData(Creature* creature, bool withDefault = true) const;

    static void BroadcastToPlayer(const Player* player, const std::string& message);
    static void AnnounceToPlayer(const Player* player, const std::string& message);
    static void AnnounceToGroup(const Player* player, const std::string& message);
    static void AnnounceToMap(const Map* map, const std::string& message);
    static void FallbackTeleport(Player* player);

    bool CanBeMythicPlus(const MapEntry* mapEntry) const;
    bool CanMapBeMythicPlus(const Map* map) const;
    bool CanProcessCreature(const Creature* creature) const;
    MythicPlusDungeonEnterState CanEnterDungeon(Map* map, const Player* player) const;
    MythicPlusDungeonEnterState CanEnterDungeonFromEntry(const MapEntry* mapEntry, const Player* player) const;
    void StoreOriginalCreatureData(Creature* creature) const;
    const MythicLevel* GetMythicLevel(uint32 level) const;
    void ProcessStaticAffixes(const MythicLevel* mythicLevel, Creature* creature) const;
    void PrintMythicLevelInfo(const MythicLevel* mythicLevel, const Player* player) const;
    bool IsInMythicPlus(const Unit* unit) const;
    bool IsMapInMythicPlus(Map* map) const;
    void LoadFromDB();
    MythicPlusDungeonInfo* GetSavedDungeonInfo(uint32 instanceId);
    void SaveDungeonInfo(uint32 instanceId, uint32 mapId, uint64 startTime, uint32 mythicLevel, bool done, bool isMythic = true);
    void AddDungeonSnapshot(uint32 instanceId, uint32 mapId, Difficulty mapDiff, uint64 startTime,
        uint64 snapTime, uint32 combatTime, uint32 charGuid, std::string charName,
        uint32 mythicLevel, uint32 creatureEntry, bool isFinalBoss, bool rewarded);
    bool IsFinalBoss(uint32 entry) const;
    void Reward(Player* player, const MythicReward& reward) const;
    void RemoveDungeonInfo(uint32 instanceId);
    const MythicLevelContainer& GetAllMythicLevels() const
    {
        return mythicLevels;
    }
    uint32 GetCurrentMythicPlusLevel(const Player* player) const;
    uint32 GetCurrentMythicPlusLevelForGUID(uint32 guid) const;
    bool SetCurrentMythicPlusLevel(const Player* player, uint32 mythiclevel);
    uint32 GetCurrentMythicPlusLevelForDungeon(const Player* player) const;
    const std::unordered_map<uint32, Difficulty>& GetAllMythicPlusDungeons() const
    {
        return mythicPlusDungeons;
    }
    void ProcessQueryCallbacks();
    void LoadMythicPlusSnapshotsFromDB();
    uint32 GetMythicSnapshotsTimer() const
    {
        return mythicSnapshotsTimer;
    }
    void UpdateMythicSnapshotsTimer(uint32 diff)
    {
        mythicSnapshotsTimer += diff;
    }
    void ResetMythicSnapshotsTimer()
    {
        mythicSnapshotsTimer = 0;
    }
    const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>> GetMapSnapshot(uint32 mapId, uint32 mythicLevel) const;
    void ProcessConfig(bool reload);
    bool IsEnabled() const
    {
        return enabled;
    }
    bool MatchMythicPlusMapDiff(const Map* map) const;
    bool IsCreatureIgnoredForMultiplyAffix(uint32 entry) const;
private:
    std::unordered_map<uint32, Difficulty> mythicPlusDungeons;
    std::unordered_map<uint32, MythicPlusDungeonInfo> mythicPlusDungeonInfo;
    std::unordered_map<uint32, uint32> charMythicLevels;
    bool enabled;
    std::set<uint32> ignoredEntriesForMultiplyAffix;

    MythicLevelContainer mythicLevels;

    QueryCallbackProcessor _queryProcessor;
    uint32 mythicSnapshotsTimer = 0;

    std::unordered_map<uint32, std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>>> dungeonMapSnapshots;

    std::unordered_map<uint32, std::vector<DBAffix>> affixesFromDB;
    std::unordered_map<uint32, std::vector<DBReward>> rewardsFromDB;

    void CreateMythicPlusDungeons();
    bool IsAllowedMythicPlusDungeon(uint32 mapId) const;
    ObjectGuid GetLeaderGuid(const Player* player) const;
    void LoadMythicPlusDungeonsFromDB();
    void LoadMythicPlusCharLevelsFromDB();
    void LoadIgnoredEntriesForMultiplyAffixFromDB();
    void MythicPlusSnapshotsDBCallback(QueryResult result);
    void SortSnapshots(std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>>& snapshots);
    void LoadMythicAffixFromDB();
    void LoadMythicRewardsFromDB();
    void LoadMythicLevelsFromDB();
};

#define sMythicPlus MythicPlus::instance()

#endif
