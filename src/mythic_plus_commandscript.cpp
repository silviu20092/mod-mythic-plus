/*
 * Credits: silviu20092
 */

#include "Chat.h"
#include "CommandScript.h"
#include "mythic_plus.h"

using namespace Acore::ChatCommands;

class mythic_plus_commandscript : public CommandScript
{
public:
    mythic_plus_commandscript() : CommandScript("mythic_plus_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable mythicCommandTable =
        {
            { "info",           HandleMythicInfoCommand,                SEC_PLAYER,             Console::No  },
            { "reload",         HandleMythicReloadCommand,              SEC_ADMINISTRATOR,      Console::Yes }
        };
        static ChatCommandTable commandTable =
        {
            { "mythic", mythicCommandTable }
        };
        return commandTable;
    }
private:
    static bool HandleMythicInfoCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (sMythicPlus->IsInMythicPlus(player))
        {
            const MythicPlus::MapData* mapData = sMythicPlus->GetMapData(player->GetMap(), false);
            ASSERT(mapData);

            const MythicLevel* level = mapData->mythicLevel;
            ASSERT(level);

            sMythicPlus->PrintMythicLevelInfo(level, player);
        }
        else
            handler->SendSysMessage("You are not in a Mythic Plus dungeon right now.");

        return true;
    }

    static bool HandleMythicReloadCommand(ChatHandler* handler)
    {
        // only load stuff that is hot reloadable
        sMythicPlus->LoadIgnoredEntriesForMultiplyAffixFromDB();
        sMythicPlus->LoadScaleMapFromDB();
        sMythicPlus->LoadSpellOverridesFromDB();
        handler->SendGlobalGMSysMessage("Hot reloadable tables were processed: mythic_plus_ignore_multiply_affix, mythic_plus_map_scale, mythic_plus_spell_override");

        return true;
    }
};

void AddSC_mythic_plus_commandscript()
{
    new mythic_plus_commandscript();
}
