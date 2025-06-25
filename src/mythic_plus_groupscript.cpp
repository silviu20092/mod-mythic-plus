/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "mythic_plus.h"

class mythic_plus_groupscript : public GroupScript
{
public:
    mythic_plus_groupscript() : GroupScript("mythic_plus_groupscript",
        {
            GROUPHOOK_ON_ADD_MEMBER
        })
    {

    }

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        ObjectGuid leaderGuid = group->GetLeaderGUID();
        if (sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter()) && guid != leaderGuid)
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(guid);
            if (player)
            {
                std::ostringstream oss;
                oss << "This group's leader has a Mythic Plus level set (level: ";
                oss << sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
                oss << ")! Group's leader can use a Mythic Keystone to transform a dungeon into Mythic Plus.";
                MythicPlus::BroadcastToPlayer(player, oss.str());
            }
        }
    }
};

void AddSC_mythic_plus_groupscript()
{
    new mythic_plus_groupscript();
}
