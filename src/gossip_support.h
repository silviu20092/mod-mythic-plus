/*
 * Credits: silviu20092
 */

#ifndef _GOSSIP_SUPPORT_H_
#define _GOSSIP_SUPPORT_H_

#include "GossipDef.h"

class Creature;

class GossipSupport
{
public:
    enum PagedDataType
    {
        PAGED_DATA_TYPE_MYTHIC_NPC_MENU,
        PAGED_DATA_TYPE_MYTHIC_LEVELS,
        PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO,
        PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST,
        PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT,
        PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS,
        PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS,
        PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS,
        PAGED_DATA_TYPE_RANDOM_AFFIXES,
        PAGED_DATA_TYPE_RANDOM_AFFIXES_FOR_LEVEL,
        MAX_PAGED_DATA_TYPE
    };

    enum IdentifierType
    {
        BASE_IDENTIFIER
    };

    struct Identifier
    {
        uint32 id;
        std::string uiName;
        GossipOptionIcon optionIcon;

        Identifier() : id(0), optionIcon(GOSSIP_ICON_INTERACT_1) {}

        virtual IdentifierType GetType() const
        {
            return BASE_IDENTIFIER;
        }
    };

    struct PagedDataCustomInfo {};

    struct PagedData
    {
        static constexpr int PAGE_SIZE = 20;

        uint32 totalPages;
        uint32 currentPage;
        PagedDataType type;
        std::vector<Identifier*> data;
        bool backMenu;
        PagedDataCustomInfo* customInfo;

        PagedData() : totalPages(0), currentPage(0), type(MAX_PAGED_DATA_TYPE), backMenu(true), customInfo(nullptr) {}

        void Reset();
        void CalculateTotals();
        void SortAndCalculateTotals(std::function<bool(const Identifier* a, const Identifier* b)> comparator);
        bool IsEmpty() const;
        const Identifier* FindIdentifierById(uint32 id) const;

        template <class T>
        T* GetCustomInfo()
        {
            if (customInfo == nullptr)
                customInfo = new T();

            return (T*)customInfo;
        }
    };
    typedef std::unordered_map<uint32, PagedData> PagedDataMap;
public:
    GossipSupport();
    ~GossipSupport();

    PagedData& GetPagedData(const Player* player);
    virtual bool AddPagedData(Player* player, Creature* creature, uint32 page);
    virtual bool TakePagedDataAction(Player* player, Creature* creature, uint32 action);
    virtual bool OnGossipHello(Player* player, Creature* creature);
    virtual bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
protected:
    PagedDataMap playerPagedData;

    virtual void NoPagedData(Player* player, const PagedData& pagedData) const;
    virtual bool _AddPagedData(Player* player, const PagedData& pagedData, uint32 page) const;
    virtual uint32 _PageZeroSender(const PagedData& pagedData) const;
    virtual void _AddMainPageInfo(Player* player, uint32 lowIndex, uint32 highIndex, const std::vector<Identifier*>& data) const;

    virtual void AddMainMenu(Player* player, Creature* creature) = 0;
};

#endif
