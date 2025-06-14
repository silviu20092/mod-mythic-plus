/*
 * Credits: silviu20092
 */

#include "Creature.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "gossip_support.h"

GossipSupport::GossipSupport()
{

}

GossipSupport::~GossipSupport()
{
    for (auto& pageData : playerPagedData)
    {
        pageData.second.Reset();
        if (pageData.second.customInfo != nullptr)
            delete pageData.second.customInfo;
    }
}

void GossipSupport::PagedData::Reset()
{
    totalPages = 0;
    backMenu = true;
    for (Identifier* identifier : data)
        delete identifier;
    data.clear();
}

void GossipSupport::PagedData::CalculateTotals()
{
    totalPages = data.size() / PAGE_SIZE;
    if (data.size() % PAGE_SIZE != 0)
        totalPages++;
}

void GossipSupport::PagedData::SortAndCalculateTotals(std::function<bool(const Identifier* a, const Identifier* b)> comparator)
{
    if (data.size() > 0)
    {
        std::sort(data.begin(), data.end(), comparator);
        CalculateTotals();
    }
}

bool GossipSupport::PagedData::IsEmpty() const
{
    return data.empty();
}

const GossipSupport::Identifier* GossipSupport::PagedData::FindIdentifierById(uint32 id) const
{
    std::vector<Identifier*>::const_iterator citer = std::find_if(data.begin(), data.end(), [&](const Identifier* idnt) { return idnt->id == id; });
    if (citer != data.end())
        return *citer;
    return nullptr;
}

GossipSupport::PagedData& GossipSupport::GetPagedData(const Player* player)
{
    return playerPagedData[player->GetGUID().GetCounter()];
}

bool GossipSupport::AddPagedData(Player* player, Creature* creature, uint32 page)
{
    ClearGossipMenuFor(player);
    PagedData& pagedData = GetPagedData(player);
    while (!_AddPagedData(player, pagedData, page))
    {
        if (page == 0)
        {
            NoPagedData(player, pagedData);
            break;
        }
        else
            page--;
    }

    pagedData.currentPage = page;

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool GossipSupport::TakePagedDataAction(Player* player, Creature* creature, uint32 action)
{
    CloseGossipMenuFor(player);
    return false;
}

void GossipSupport::NoPagedData(Player* player, const PagedData& pagedData) const
{
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cffb50505NOTHING ON THIS PAGE|r", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- [First Page]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
}

bool GossipSupport::_AddPagedData(Player* player, const PagedData& pagedData, uint32 page) const
{
    const std::vector<Identifier*>& data = pagedData.data;
    if (data.size() == 0 || (page + 1) > pagedData.totalPages)
        return false;

    uint32 lowIndex = page * PagedData::PAGE_SIZE;
    if (data.size() <= lowIndex)
        return false;

    uint32 highIndex = lowIndex + PagedData::PAGE_SIZE - 1;
    if (highIndex >= data.size())
        highIndex = data.size() - 1;

    _AddMainPageInfo(player, lowIndex, highIndex, data);

    if (page + 1 < pagedData.totalPages)
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Next] ->", GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF + page + 1);

    if (pagedData.backMenu)
    {
        uint32 pageZeroSender = _PageZeroSender(pagedData);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- [Back]", page == 0 ? pageZeroSender : GOSSIP_SENDER_MAIN + 2, page == 0 ? GOSSIP_ACTION_INFO_DEF : GOSSIP_ACTION_INFO_DEF + page - 1);
    }

    return true;
}

void GossipSupport::_AddMainPageInfo(Player* player, uint32 lowIndex, uint32 highIndex, const std::vector<Identifier*>& data) const
{
    for (uint32 i = lowIndex; i <= highIndex; i++)
    {
        const Identifier* identifier = data[i];
        AddGossipItemFor(player, identifier->optionIcon, identifier->uiName, GOSSIP_SENDER_MAIN + 1, GOSSIP_ACTION_INFO_DEF + identifier->id);
    }
}

uint32 GossipSupport::_PageZeroSender(const PagedData& pagedData) const
{
    return GOSSIP_SENDER_MAIN;
}

bool GossipSupport::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (sender == GOSSIP_SENDER_MAIN)
        return OnGossipHello(player, creature);
    else if (sender == GOSSIP_SENDER_MAIN + 1)
    {
        uint32 id = action - GOSSIP_ACTION_INFO_DEF;
        return TakePagedDataAction(player, creature, id);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 2)
    {
        uint32 page = action - GOSSIP_ACTION_INFO_DEF;
        return AddPagedData(player, creature, page);
    }

    return false;
}

bool GossipSupport::OnGossipHello(Player* player, Creature* creature)
{
    AddMainMenu(player, creature);
    return AddPagedData(player, creature, 0);
}
