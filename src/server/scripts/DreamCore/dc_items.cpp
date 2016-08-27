#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "dc_items.h"
#include "DCMgr.h"

class DCItemScript : public ItemScript
{
    public:
        DCItemScript() :
            ItemScript("dc_item")
        {
        }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
        {
            auto dcItem = sDCMgr->GetDCItem(item->GetEntry());
            if (!dcItem)
            {
                TC_LOG_ERROR("dc.items", "Client send DCItem [%u]. but this item has not been stored in dc_items, ignored.", item->GetEntry());
                return false;
            }

            ChatHandler handler(player->GetSession());

            auto itemName = item->GetTemplate()->Name1;
            if (auto il = sObjectMgr->GetItemLocale(item->GetEntry()))
                ObjectMgr::GetLocaleString(il->Name, handler.GetSessionDbLocaleIndex(), itemName);

            switch (dcItem->type)
            {
                case DC_ITEM_TYPE_SCROLL_XP:
                {
                    player->DestroyItemCount(item->GetEntry(), 1, true);
                    player->GiveXP(dcItem->data0, dcItem->data1, dcItem->data2);
                    handler.SendSysMessage(sDCMgr->BuildDCText(STRING_YOU_USE_AND_GET_XP, itemName.c_str(), uint32(dcItem->data0)).c_str());
                    return true;
                }
                case DC_ITEM_TYPE_SCROLL_PROFESSION:
                {
                    auto skill = uint32(dcItem->data0);
                    SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skill);
                    if (!skillLine)
                    {
                        TC_LOG_ERROR("dc.items", "Client send DCItem [%u]. type [%u], but skill [%i] dose not exist, ignored.", item->GetEntry(), dcItem->type, skill);
                        return false;
                    }

                    if (!player->HasSkill(skill))
                    {
                        handler.SendSysMessage(sDCMgr->BuildDCText(STRING_YOU_DONT_KNOW_THIS_SKILL, skillLine->name[handler.GetSessionDbcLocale()]).c_str());
                        return true;
                    }

                    if (player->GetPureSkillValue(skill) >= player->GetPureMaxSkillValue(skill))
                    {
                        handler.SendSysMessage(sDCMgr->BuildDCText(STRING_SKILL_ALREADY_MAX).c_str());
                        return true;
                    }

                    auto curVal = player->GetSkillValue(skill);
                    auto maxVal = player->GetMaxSkillValue(skill);
                    auto newVal = curVal + dcItem->data1;

                    if (newVal > maxVal)
                        newVal = maxVal;

                    player->DestroyItemCount(item->GetEntry(), 1, true);
                    player->SetSkill(skill, player->GetSkillStep(skill), newVal, maxVal);

                    handler.SendSysMessage(sDCMgr->BuildDCText(STRING_YOU_USE_AND_GET_PROFESSION, itemName.c_str(), uint32(dcItem->data1), skillLine->name[handler.GetSessionDbcLocale()]).c_str());
                    return true;
                }
                case DC_ITEM_TYPE_SCROLL_TELEPORT:
                {
                    return true;
                }
                case DC_ITEM_TYPE_SCROLL_UNBINDINST:
                {
                    player->PlayerTalkClass->ClearMenus();

                    auto  count = 0u;
                    auto& binds = player->GetBoundInstances(Difficulty(DUNGEON_DIFFICULTY_HEROIC));
                    for (auto& itr : binds)
                    {
                        auto save = itr.second.save;
                        auto map  = sMapStore.LookupEntry(itr.first);
                        if (map->IsRaid())
                            continue;

                        std::string mapName("|cff00ff00|TInterface\\icons\\achievement_zone_ironforge:40:40:-15:0|t|r");
                        mapName.append(map->name[4]);
                        player->ADD_GOSSIP_ITEM_EXTENDED(8, mapName, GOSSIP_SENDER_MAIN, itr.first, sDCMgr->BuildDCText(STRING_INST_RESET_REMIND, map->name[4]), 0, 0);

                        ++count;
                    }

                    player->SEND_GOSSIP_MENU(count ? 10000002 : 10000003, item->GetGUID());

                    return true;
                }
                default:
                {
                   TC_LOG_ERROR("dc.items", "Client send DCItem [%u], type [%u], but this type dose not exist, ignored.", item->GetEntry(), dcItem->type);
                   return false;
                }
            }
        }

        bool OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
        {
            player->CLOSE_GOSSIP_MENU();
            player->PlayerTalkClass->ClearMenus();

            auto dcItem = sDCMgr->GetDCItem(item->GetEntry());
            if (!dcItem)
            {
                TC_LOG_ERROR("dc.items", "Client send DCItem [%u] GossipSelect. but this item has not been stored in dc_items, ignored.", item->GetEntry());
                return false;
            }

            switch (dcItem->type)
            {
                case DC_ITEM_TYPE_SCROLL_UNBINDINST:
                {
                    auto map = sMapStore.LookupEntry(action);
                    if (!map)
                    {
                        TC_LOG_ERROR("dc.items", "Client try to unbind map[%u]. but this map dose not exist, ignored.", action);
                        return false;
                    }

                    if (action == player->GetMapId())
                    {
                        player->GetSession()->SendNotification(sDCMgr->BuildDCText(STRING_INST_RESET_FAILES_IN_INST).c_str());
                        return true;
                    }

                    ChatHandler handler(player->GetSession());

                    auto itemName = item->GetTemplate()->Name1;
                    if (auto il = sObjectMgr->GetItemLocale(item->GetEntry()))
                        ObjectMgr::GetLocaleString(il->Name, handler.GetSessionDbLocaleIndex(), itemName);

                    player->DestroyItemCount(item->GetEntry(), 1, true);
                    player->UnbindInstance(action, Difficulty(DUNGEON_DIFFICULTY_HEROIC));

                    handler.SendSysMessage(sDCMgr->BuildDCText(STRING_YOU_USE_AND_RESET_INST, itemName.c_str(), map->name[4]).c_str());
                    return true;
                }
                default:
                {
                   TC_LOG_ERROR("dc.items", "Client send DCItem [%u] GossipSelect, type [%u], but this type dose not exist or this item dose not have gossip menu, ignored.", item->GetEntry(), dcItem->type);
                   return false;
                }
            }
        }

        bool OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) override
        {
            return true;
        }
};

void AddSC_DC_items()
{
    new DCItemScript();
}
