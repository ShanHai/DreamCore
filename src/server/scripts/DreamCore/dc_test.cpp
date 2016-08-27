#include "dc_test.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"

class TestItem : public ItemScript
{
    public:
        TestItem():
            ItemScript("dc_test_item")
    {
    }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        player->ADD_GOSSIP_ITEM(0, "|cff00ff00|TInterface\\icons\\spell_holy_fanaticism:50:50:-20:0|t|rHello, ChgTrinity!", 1, 100);
        player->ADD_GOSSIP_ITEM(0, "|cff00ff00|TInterface\\icons\\spell_holy_fanaticism:50:50:-20:0|t|rHello, ChgTrinity!", 2, 100);
        player->ADD_GOSSIP_ITEM(0, "|cff00ff00|TInterface\\icons\\spell_holy_fanaticism:50:50:-20:0|t|rHello, ChgTrinity!", 3, 100);
        player->ADD_GOSSIP_ITEM(0, "|cff00ff00|TInterface\\icons\\spell_holy_fanaticism:40:40:-20:0|t|rHello, ChgTrinity!", 4, 100);
        player->SEND_GOSSIP_MENU(907, item->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();
        if (sender == 1)
        {
            player->SetLevel(10);
            return true;
        }
        else if (sender == 2)
        {
            player->SetLevel(20);
            return true;
        }
        else if (sender == 3)
        {
            player->SetLevel(30);
            return true;
        }

        return false;
    }

    bool OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code)
    {
        return true;
    }
};

void AddSC_DC_test()
{
    new TestItem();
}
