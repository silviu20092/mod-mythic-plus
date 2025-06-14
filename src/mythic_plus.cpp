/*
 * Credits: silviu20092
 */

#include "AreaDefines.h"
#include "Chat.h"
#include "Group.h"
#include "Config.h"
#include "mythic_affix.h"
#include "mythic_plus.h"

MythicPlus::MythicPlus()
{
    enabled = true;
    CreateMythicPlusDungeons();
    CreateMythicLevels();
}

MythicPlus::~MythicPlus()
{
    for (auto& mythicLevel : mythicLevels)
        for (auto* affix : mythicLevel.affixes)
            delete affix;
}

MythicPlus* MythicPlus::instance()
{
    static MythicPlus instance;
    return &instance;
}

MythicPlus::MapData* MythicPlus::GetMapData(Map* map, bool withDefault) const
{
    if (withDefault)
        return map->CustomData.GetDefault<MapData>("mythicPlusMapData");

    return map->CustomData.Get<MapData>("mythicPlusMapData");
}

MythicPlus::CreatureData* MythicPlus::GetCreatureData(Creature* creature, bool withDefault) const
{
    if (withDefault)
        return creature->CustomData.GetDefault<CreatureData>("mythicPlusCreatureData");

    return creature->CustomData.Get<CreatureData>("mythicPlusCreatureData");
}

/*static*/ void MythicPlus::BroadcastToPlayer(const Player* player, const std::string& message)
{
    std::ostringstream oss;
    oss << "|cffa11585[MYTHIC PLUS]|r: ";
    oss << message;
    ChatHandler(player->GetSession()).SendSysMessage(oss.str());
}

/*static*/ void MythicPlus::AnnounceToPlayer(const Player* player, const std::string& message)
{
    ChatHandler(player->GetSession()).SendNotification(message);
}

/*static*/ void MythicPlus::AnnounceToGroup(const Player* player, const std::string& message)
{
    const Group* group = player->GetGroup();
    if (group == nullptr)
    {
        AnnounceToPlayer(player, message);
        return;
    }

    for (const GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
    {
        const Player* groupPlayer = groupRef->GetSource();
        if (groupPlayer && groupPlayer->IsInWorld())
            AnnounceToPlayer(groupPlayer, message);
    }
}

/*static*/ void MythicPlus::AnnounceToMap(const Map* map, const std::string& message)
{
    Map::PlayerList const& playerList = map->GetPlayers();
    if (playerList.IsEmpty())
        return;

    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
        if (Player* player = itr->GetSource())
            AnnounceToPlayer(player, message);
}

/*static*/ void MythicPlus::FallbackTeleport(Player* player)
{
    player->TeleportToEntryPoint();
}

bool MythicPlus::CanBeMythicPlus(const MapEntry* mapEntry) const
{
    if (!mapEntry)
        return false;

    if (!mapEntry->IsNonRaidDungeon())
        return false;

    return IsAllowedMythicPlusDungeon(mapEntry->MapID);
}

bool MythicPlus::CanMapBeMythicPlus(const Map* map) const
{
    return IsEnabled() && MatchMythicPlusMapDiff(map);
}

bool MythicPlus::CanProcessCreature(const Creature* creature) const
{
    if (!creature)
        return false;

    Map* map = creature->GetMap();
    if (!map || !map->IsNonRaidDungeon() || !map->ToInstanceMap())
        return false;

    if (creature->GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if ((creature->IsHunterPet() || creature->IsPet() || creature->IsSummon()) && creature->IsControlledByPlayer())
        return false;

    if (creature->IsCritter() || creature->IsTotem() || creature->IsTrigger())
        return false;

    if (creature->ToTempSummon() &&
        creature->ToTempSummon()->GetSummoner() &&
        creature->ToTempSummon()->GetSummoner()->ToPlayer())
        return false;

    return true;
}

MythicPlus::MythicPlusDungeonEnterState MythicPlus::CanEnterDungeon(Map* map, const Player* player) const
{
    if (!CanMapBeMythicPlus(map))
        return ENTER_STATE_OK;

    const MythicPlusDungeonInfo* savedDungeon = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());
    ASSERT(savedDungeon);

    // if no Mythic Plus level was set, just allow
    if (savedDungeon->mythicLevel == 0)
        return ENTER_STATE_OK;

    // player must be in a group and the leader must be online in order to join M+
    ObjectGuid leaderGuid = GetLeaderGuid(player);
    if (leaderGuid.IsEmpty())
        return ENTER_STATE_NO_GROUP_LEADER;

    Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid);
    if (!leader)
        return ENTER_STATE_LEADER_OFFLINE;

    return ENTER_STATE_OK;
}

MythicPlus::MythicPlusDungeonEnterState MythicPlus::CanEnterDungeonFromEntry(const MapEntry* mapEntry, const Player* player) const
{
    if (!CanBeMythicPlus(mapEntry))
        return ENTER_STATE_OK;

    // allow entry if no group
    ObjectGuid leaderGuid = GetLeaderGuid(player);
    if (leaderGuid.IsEmpty())
        return ENTER_STATE_OK;

    // first, check if the map can actually start a M+ dungeon
    Difficulty playerDiff = player->GetDifficulty(false);
    Difficulty dungeonDiff = mythicPlusDungeons.at(mapEntry->MapID);
    if (playerDiff < dungeonDiff)
        return ENTER_STATE_OK;

    uint32 leaderMythicLevel = GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
    if (leaderMythicLevel == 0)
        return ENTER_STATE_OK;

    // let's just wait for the leader to come online
    Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid);
    if (!leader)
        return ENTER_STATE_LEADER_OFFLINE;

    return ENTER_STATE_OK;
}

void MythicPlus::StoreOriginalCreatureData(Creature* creature) const
{
    CreatureData* creatureData = GetCreatureData(creature);
    creatureData->originalCreateHealth = creature->GetCreateHealth();
    creatureData->originalMaxHealth = creature->GetMaxHealth();
}

const MythicLevel* MythicPlus::GetMythicLevel(uint32 level) const
{
    MythicLevelContainer::const_iterator itr = std::find_if(mythicLevels.begin(), mythicLevels.end(), [&level](const MythicLevel& mythicLevel) { return mythicLevel.level == level; });
    if (itr == mythicLevels.end())
        return nullptr;

    return &*itr;
}

void MythicPlus::ProcessStaticAffixes(const MythicLevel* mythicLevel, Creature* creature) const
{
    for (auto* affix : mythicLevel->affixes)
        affix->HandleStaticEffect(creature);
}

void MythicPlus::PrintMythicLevelInfo(const MythicLevel* mythicLevel, const Player* player) const
{
    ChatHandler handler(player->GetSession());
    handler.PSendSysMessage("Current Mythic Plus level is {}", mythicLevel->level);
    handler.PSendSysMessage("Active affix count: {}", mythicLevel->affixes.size());
    uint32 counter = 1;
    for (const auto* affix : mythicLevel->affixes)
        handler.PSendSysMessage("    {}. {}", counter++, affix->ToString());
}

bool MythicPlus::IsInMythicPlus(const Unit* unit) const
{
    return IsMapInMythicPlus(unit->GetMap());
}

bool MythicPlus::IsMapInMythicPlus(Map* map) const
{
    MapData* mapData = GetMapData(map, false);
    return mapData != nullptr && mapData->mythicLevel != nullptr;
}

void MythicPlus::LoadFromDB()
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    trans->Append("DELETE FROM `mythic_plus_dungeon` WHERE `id` NOT IN (SELECT `id` FROM `instance`)");
    trans->Append("DELETE FROM `mythic_plus_char_level` WHERE `guid` NOT IN (SELECT `guid` FROM `characters`)");
    CharacterDatabase.DirectCommitTransaction(trans);

    LoadMythicPlusDungeonsFromDB();
    LoadMythicPlusCharLevelsFromDB();
    LoadIgnoredEntriesForMultiplyAffixFromDB();
}

MythicPlus::MythicPlusDungeonInfo* MythicPlus::GetSavedDungeonInfo(uint32 instanceId)
{
    if (mythicPlusDungeonInfo.find(instanceId) != mythicPlusDungeonInfo.end())
        return &mythicPlusDungeonInfo.at(instanceId);

    return nullptr;
}

void MythicPlus::SaveDungeonInfo(uint32 instanceId, uint32 mapId, uint64 startTime, uint32 mythicLevel, bool done, bool isMythic)
{
    CharacterDatabase.Execute("REPLACE INTO mythic_plus_dungeon (id, map, starttime, mythiclevel, done, ismythic) VALUES ({}, {}, {}, {}, {}, {})",
        instanceId, mapId, startTime, mythicLevel, done, isMythic);

    MythicPlusDungeonInfo* dungeonInfo = GetSavedDungeonInfo(instanceId);
    if (dungeonInfo == nullptr)
    {
        MythicPlusDungeonInfo dungeonInfo;
        dungeonInfo.instanceId = instanceId;
        dungeonInfo.mapId = mapId;
        dungeonInfo.startTime = startTime;
        dungeonInfo.mythicLevel = mythicLevel;
        dungeonInfo.done = done;
        dungeonInfo.isMythic = isMythic;
        mythicPlusDungeonInfo[instanceId] = dungeonInfo;
    }
    else
    {
        dungeonInfo->startTime = startTime;
        dungeonInfo->done = done;
    }
}

void MythicPlus::AddDungeonSnapshot(uint32 instanceId, uint32 mapId, Difficulty mapDiff, uint64 startTime,
    uint64 snapTime, uint32 combatTime, uint32 charGuid, std::string charName,
    uint32 mythicLevel, uint32 creatureEntry, bool isFinalBoss, bool rewarded)
{
    CharacterDatabase.Execute("INSERT INTO mythic_plus_dungeon_snapshot (id, map, mapdifficulty, starttime, snaptime, combattime, char_guid, char_name, mythiclevel, creature_entry, creature_final_boss, rewarded) VALUES "
        "({}, {}, {}, {}, {}, {}, {}, \"{}\", {}, {}, {}, {})", instanceId, mapId, mapDiff, startTime, snapTime, combatTime, charGuid, charName, mythicLevel, creatureEntry, isFinalBoss, rewarded);
}

void MythicPlus::CreateMythicPlusDungeons()
{
    mythicPlusDungeons[MAP_PIT_OF_SARON] = Difficulty::DUNGEON_DIFFICULTY_NORMAL;
    mythicPlusDungeons[MAP_THE_FORGE_OF_SOULS] = Difficulty::DUNGEON_DIFFICULTY_NORMAL;

    mythicPlusDungeons[MAP_AHN_KAHET_THE_OLD_KINGDOM] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_AZJOL_NERUB] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_DRAK_THARON_KEEP] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_GUNDRAK] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_HALLS_OF_LIGHTNING] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_HALLS_OF_STONE] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_THE_NEXUS] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_THE_OCULUS] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_UTGARDE_KEEP] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
    mythicPlusDungeons[MAP_UTGARDE_PINNACLE] = Difficulty::DUNGEON_DIFFICULTY_HEROIC;
}

bool MythicPlus::IsAllowedMythicPlusDungeon(uint32 mapId) const
{
    if (!IsEnabled())
        return false;

    return mythicPlusDungeons.find(mapId) != mythicPlusDungeons.end();
}

ObjectGuid MythicPlus::GetLeaderGuid(const Player* player) const
{
    const Group* group = player->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    return group->GetLeaderGUID();
}

void MythicPlus::CreateMythicLevels()
{
    MythicLevel level1;
    level1.level = 1;
    level1.affixes.push_back(new TrashHealthIncreaseAffix(0.15f));
    level1.timeLimit = 60 * 45;
    level1.reward.money = 1000000;
    level1.reward.AddToken(29434, 1);
    mythicLevels.push_back(level1);

    MythicLevel level2;
    level2.level = 2;
    level2.affixes.push_back(new TrashHealthIncreaseAffix(0.15f));
    level2.affixes.push_back(new BossHealthIncreaseAffix(0.1f));
    level2.affixes.push_back(new MoreDamageForCreaturesAffix(10.0f));
    level2.timeLimit = 60 * 45;
    level2.reward.money = 4000000;
    level2.reward.AddToken(29434, 2);
    mythicLevels.push_back(level2);

    MythicLevel level3;
    level3.level = 3;
    level3.affixes.push_back(new TrashHealthIncreaseAffix(0.2f));
    level3.affixes.push_back(new BossHealthIncreaseAffix(0.15f));
    level3.affixes.push_back(new MultipleEnemiesAffix(15.0f));
    level3.affixes.push_back(new MoreDamageForCreaturesAffix(20.0f));
    level3.timeLimit = 60 * 40;
    level3.reward.money = 8000000;
    level3.reward.AddToken(29434, 4);
    mythicLevels.push_back(level3);

    MythicLevel level4;
    level4.level = 4;
    level4.affixes.push_back(new TrashHealthIncreaseAffix(0.2f));
    level4.affixes.push_back(new BossHealthIncreaseAffix(0.15f));
    level4.affixes.push_back(new MultipleEnemiesAffix(30.0f));
    level4.affixes.push_back(new MoreDamageForCreaturesAffix(30.0f));
    level4.timeLimit = 60 * 40;
    level4.reward.money = 10000000;
    level4.reward.AddToken(29434, 7);
    mythicLevels.push_back(level4);

    MythicLevel level5;
    level5.level = 5;
    level5.affixes.push_back(new TrashHealthIncreaseAffix(0.2f));
    level5.affixes.push_back(new BossHealthIncreaseAffix(0.15f));
    level5.affixes.push_back(new MultipleEnemiesAffix(30.0f));
    level5.affixes.push_back(new MoreDamageForCreaturesAffix(30.0f));
    level5.affixes.push_back(new RandomlyExplodeAffix());
    level5.timeLimit = 60 * 40;
    level5.reward.money = 15000000;
    level5.reward.AddToken(29434, 10);
    mythicLevels.push_back(level5);
}

void MythicPlus::LoadMythicPlusDungeonsFromDB()
{
    mythicPlusDungeonInfo.clear();

    QueryResult result = CharacterDatabase.Query("SELECT id, map, starttime, mythiclevel, done, ismythic FROM mythic_plus_dungeon");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint32 instanceId = fields[0].Get<uint32>();
        uint32 mapId = fields[1].Get<uint32>();
        uint64 startTime = fields[2].Get<uint64>();
        uint32 mythicLevel = fields[3].Get<uint32>();
        bool done = fields[4].Get<bool>();
        bool isMythic = fields[5].Get<bool>();

        MythicPlusDungeonInfo dungeonInfo;
        dungeonInfo.instanceId = instanceId;
        dungeonInfo.mapId = mapId;
        dungeonInfo.startTime = startTime;
        dungeonInfo.mythicLevel = mythicLevel;
        dungeonInfo.done = done;
        dungeonInfo.isMythic = isMythic;

        mythicPlusDungeonInfo[instanceId] = dungeonInfo;
    } while (result->NextRow());
}

void MythicPlus::LoadMythicPlusCharLevelsFromDB()
{
    charMythicLevels.clear();

    QueryResult result = CharacterDatabase.Query("SELECT guid, mythiclevel FROM mythic_plus_char_level");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint32 guid = fields[0].Get<uint32>();
        uint32 mythiclevel = fields[1].Get<uint32>();

        charMythicLevels[guid] = mythiclevel;
    } while (result->NextRow());
}

void MythicPlus::LoadIgnoredEntriesForMultiplyAffixFromDB()
{
    ignoredEntriesForMultiplyAffix.clear();

    QueryResult result = WorldDatabase.Query("SELECT entry FROM mythic_plus_ignore_multiply_affix");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].Get<uint32>();
        if (sObjectMgr->GetCreatureTemplate(entry))
            ignoredEntriesForMultiplyAffix.insert(entry);
    } while (result->NextRow());
}

void MythicPlus::LoadMythicPlusSnapshotsFromDB()
{
    std::string query =
        "select "
            "mpds.id, "
            "mpds.starttime, "
            "mpds.snaptime, "
            "mpds.creature_entry, "
            "mpds.map, "
            "group_concat(mpds.char_name) players, "
            "max(mpds.combattime) combattime, "
            "max(mythiclevel) mythiclevel, "
            "max(creature_final_boss) creature_final_boss, "
            "max(rewarded) rewarded, "
            "mpds.mapdifficulty "
        "from mythic_plus_dungeon_snapshot mpds "
        "group by mpds.id, mpds.map, mpds.mapdifficulty, mpds.starttime, mpds.snaptime, mpds.creature_entry";
    _queryProcessor.AddCallback(CharacterDatabase.AsyncQuery(query).WithCallback(std::bind(&MythicPlus::MythicPlusSnapshotsDBCallback, this, std::placeholders::_1)));
}

/*static*/ std::string MythicPlus::Utils::GetCreatureName(const Player* player, const Creature* creature)
{
    return creature->GetNameForLocaleIdx(player->GetSession()->GetSessionDbLocaleIndex());
}

/*static*/ std::string MythicPlus::Utils::GetCreatureNameByEntry(const Player* player, uint32 entry)
{
    auto creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
    auto creatureLocale = sObjectMgr->GetCreatureLocale(entry);
    std::string name;

    if (creatureLocale)
        name = creatureLocale->Name[player->GetSession()->GetSessionDbcLocale()];

    if (name.empty() && creatureTemplate)
        name = creatureTemplate->Name;

    if (name.empty())
        name = "Unknown creature";

    return name;
}

/*static*/ uint32 MythicPlus::Utils::PlayerGUID(const Player* player)
{
    return player->GetGUID().GetCounter();
}

/*static*/ std::string MythicPlus::Utils::CopperToMoneyStr(uint32 money, bool colored)
{
    uint32 gold = money / GOLD;
    uint32 silver = (money % GOLD) / SILVER;
    uint32 copper = (money % GOLD) % SILVER;

    std::ostringstream oss;
    if (gold > 0)
    {
        if (colored)
            oss << gold << "|cffb3aa34g|r";
        else
            oss << gold << "g";
    }
    if (silver > 0)
    {
        if (colored)
            oss << silver << "|cff7E7C7Fs|r";
        else
            oss << silver << "s";
    }
    if (copper > 0)
    {
        if (colored)
            oss << copper << "|cff974B29c|r";
        else
            oss << copper << "c";
    }

    return oss.str();
}

/*static*/ std::string MythicPlus::Utils::ItemIcon(const ItemTemplate* proto, uint32 width, uint32 height, int x, int y)
{
    std::ostringstream ss;
    ss << "|TInterface";
    const ItemDisplayInfoEntry* dispInfo = nullptr;
    if (proto)
    {
        dispInfo = sItemDisplayInfoStore.LookupEntry(proto->DisplayInfoID);
        if (dispInfo)
            ss << "/ICONS/" << dispInfo->inventoryIcon;
    }
    if (!dispInfo)
        ss << "/InventoryItems/WoWUnknownItem01";
    ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
    return ss.str();
}

/*static*/ std::string MythicPlus::Utils::ItemIcon(const ItemTemplate* proto)
{
    return ItemIcon(proto, 30, 30, 0, 0);
}

/*static*/ std::string MythicPlus::Utils::ItemName(const ItemTemplate* proto, const Player* player)
{
    LocaleConstant loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
    std::string name = proto->Name1;
    if (ItemLocale const* il = sObjectMgr->GetItemLocale(proto->ItemId))
        ObjectMgr::GetLocaleString(il->Name, loc_idx, name);
    return name;
}

/*static*/ std::string MythicPlus::Utils::ItemLinkForUI(uint32 entry, const Player* player)
{
    const ItemTemplate* proto = sObjectMgr->GetItemTemplate(entry);
    std::ostringstream oss;
    oss << ItemIcon(proto);
    oss << ItemName(proto, player);
    return oss.str();
}

/*static*/ std::string MythicPlus::Utils::Colored(const std::string& message, const std::string& hexColor)
{
    std::ostringstream oss;
    oss << "|cff" << hexColor;
    oss << message;
    oss << "|r";
    return oss.str();
}

/*static*/ std::string MythicPlus::Utils::RedColored(const std::string& message)
{
    return Colored(message, "b50505");
}

/*static*/ std::string MythicPlus::Utils::GreenColored(const std::string& message)
{
    return Colored(message, "0d852d");
}

/*static*/ void MythicPlus::Utils::VisualFeedback(Player* player)
{
    player->CastSpell(player, VISUAL_FEEDBACK_SPELL_ID, true);
}

/*static*/ std::string MythicPlus::Utils::FormatFloat(float val, uint32 decimals)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << val;
    return oss.str();
}

/*static*/ std::string MythicPlus::Utils::DateFromSeconds(uint64 seconds)
{
    std::time_t secs = seconds;
    std::tm* utc_tm = std::gmtime(&secs);
    std::ostringstream oss;
    oss << std::put_time(utc_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool MythicPlus::IsFinalBoss(uint32 entry) const
{
    return entry == 36502       // Forge of Souls
        || entry == 36658       // Pit of Saron
        || entry == 29306       // Gundrak
        || entry == 29311       // Ahn'Kahet
        || entry == 29120       // Azjol Nerub
        || entry == 26632       // DrakTharon
        || entry == 28923       // Halls of Lightning
        || entry == 27978       // Halls of Stone
        || entry == 26723       // The Nexus
        || entry == 27656       // The Oculus
        || entry == 23954       // Utgarde Keep
        || entry == 26861;      // Utgarde Pinnacle
}

void MythicReward::AddToken(uint32 entry, uint32 count)
{
    tokens.push_back(std::make_pair(entry, count));
}

void MythicPlus::Reward(Player* player, const MythicReward& reward) const
{
    if (reward.money)
        player->ModifyMoney(reward.money);

    for (auto& token : reward.tokens)
        player->AddItem(token.first, token.second);
}

void MythicPlus::RemoveDungeonInfo(uint32 instanceId)
{
    std::unordered_map<uint32, MythicPlusDungeonInfo>::iterator itr = mythicPlusDungeonInfo.find(instanceId);
    if (itr != mythicPlusDungeonInfo.end())
    {
        mythicPlusDungeonInfo.erase(itr);
        CharacterDatabase.Execute("DELETE FROM mythic_plus_dungeon WHERE id = {}", instanceId);
    }
}

uint32 MythicPlus::GetCurrentMythicPlusLevel(const Player* player) const
{
    return GetCurrentMythicPlusLevelForGUID(Utils::PlayerGUID(player));
}

uint32 MythicPlus::GetCurrentMythicPlusLevelForGUID(uint32 guid) const
{
    if (charMythicLevels.find(guid) != charMythicLevels.end())
        return charMythicLevels.at(guid);

    return 0;
}

bool MythicPlus::SetCurrentMythicPlusLevel(const Player* player, uint32 mythiclevel)
{
    if (player->GetGroup() == nullptr)
    {
        uint32 guid = Utils::PlayerGUID(player);
        charMythicLevels[guid] = mythiclevel;
        CharacterDatabase.Execute("REPLACE INTO mythic_plus_char_level (guid, mythiclevel) VALUES ({}, {})", guid, mythiclevel);
        return true;
    }

    return false;
}

uint32 MythicPlus::GetCurrentMythicPlusLevelForDungeon(const Player* player) const
{
    ObjectGuid leaderGuid = GetLeaderGuid(player);
    if (leaderGuid.IsEmpty())
        return 0;

    Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid);
    if (leader == nullptr)
        return 0;

    return GetCurrentMythicPlusLevel(leader);
}

void MythicPlus::ProcessQueryCallbacks()
{
    _queryProcessor.ProcessReadyCallbacks();
}

void MythicPlus::MythicPlusSnapshotsDBCallback(QueryResult result)
{
    dungeonMapSnapshots.clear();

    if (!result)
        return;

    std::unordered_map<uint32, std::map<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>> mapSnapshots;
    do
    {
        Field* fields = result->Fetch();

        MythicPlusDungeonSnapshot snapshot;
        snapshot.id = fields[0].Get<uint32>();
        snapshot.startTime = fields[1].Get<uint64>();
        snapshot.snapTime = fields[2].Get<uint64>();
        snapshot.entry = fields[3].Get<uint32>();
        snapshot.mapId = fields[4].Get<uint32>();
        snapshot.players = fields[5].Get<std::string>();
        snapshot.combatTime = fields[6].Get<uint32>();
        snapshot.mythicLevel = fields[7].Get<uint32>();
        snapshot.finalBoss = fields[8].Get<bool>();
        snapshot.rewarded = fields[9].Get<bool>();
        snapshot.difficulty = fields[10].Get<bool>();
        mapSnapshots[snapshot.mapId][std::make_pair(snapshot.id, snapshot.startTime)].push_back(snapshot);
    } while (result->NextRow());

    for (auto& mpair : mapSnapshots)
    {
        std::map<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>& snapshots = mpair.second;
        for (auto& spair : snapshots)
        {
            std::vector<MythicPlusDungeonSnapshot>& snaps = spair.second;
            uint32 totalTime = 0;
            uint32 endTime = 0;
            bool rewarded = false;
            for (const auto& s : snaps)
            {
                if (s.finalBoss)
                {
                    rewarded = s.rewarded;
                    totalTime = s.snapTime - s.startTime;
                    endTime = s.snapTime;
                }
            }
            for (auto& s : snaps)
            {
                s.rewarded = rewarded;
                s.totalTime = totalTime;
                s.endTime = endTime;
            }
        }

        std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>> sortedSnapshots(snapshots.begin(), snapshots.end());
        SortSnapshots(sortedSnapshots);

        dungeonMapSnapshots[mpair.first] = sortedSnapshots;
    }
}

const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> MythicPlus::GetMapSnapshot(uint32 mapId, uint32 mythicLevel) const
{
    std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> result;
    if (dungeonMapSnapshots.find(mapId) != dungeonMapSnapshots.end())
    {
        const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>>* snaps = &dungeonMapSnapshots.at(mapId);
        for (const auto& s : *snaps)
            if (mythicLevel == 0 || s.second.at(0).mythicLevel == mythicLevel)
                result.push_back(s);
    }

    return result;
}

void MythicPlus::SortSnapshots(std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlusDungeonSnapshot>>>& snapshots)
{
    std::sort(snapshots.begin(), snapshots.end(), [](const auto& a, const auto& b) {
        const std::vector<MythicPlusDungeonSnapshot>& snapA = a.second;
        const std::vector<MythicPlusDungeonSnapshot>& snapB = b.second;
        uint32 snapATime = snapA.at(0).totalTime;
        uint32 snapBTime = snapB.at(0).totalTime;
        if (snapATime == 0 && snapBTime == 0)
            return false;
        if (snapATime == 0)
            return false;
        if (snapBTime == 0)
            return true;

        return snapATime < snapBTime;
    });

    uint32 internalId = 0;
    for (auto& s : snapshots)
    {
        internalId++;
        for (auto& e : s.second)
            e.internalId = internalId;
    }
}

void MythicPlus::ProcessConfig(bool reload)
{
    if (!reload)
        enabled = sConfigMgr->GetOption<bool>("MythicPlus.Enable", true);
}

bool MythicPlus::MatchMythicPlusMapDiff(const Map* map) const
{
    if (!map || !map->GetEntry())
        return false;

    if (!map->IsNonRaidDungeon())
        return false;

    if (mythicPlusDungeons.find(map->GetId()) != mythicPlusDungeons.end())
    {
        Difficulty mapDiff = map->GetDifficulty();
        Difficulty dungeonDiff = mythicPlusDungeons.at(map->GetId());
        return mapDiff >= dungeonDiff;
    }

    return false;
}

bool MythicPlus::IsCreatureIgnoredForMultiplyAffix(uint32 entry) const
{
    return ignoredEntriesForMultiplyAffix.find(entry) != ignoredEntriesForMultiplyAffix.end();
}
