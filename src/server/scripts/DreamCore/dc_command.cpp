#include "AccountMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "DCMgr.h"
#include "DCVIPModule.h"

class DCCommand : public CommandScript
{
public:
    DCCommand():
        CommandScript("dc_commandscript")
    {
    }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> dcVipAddCommandTable =
        {
            { "vendor", rbac::RBAC_PERM_COMMAND_DC_VIP_ADD_VENDOR,  true,  &HandleDCVipAddVendorCommand,    "" },
        };
        static std::vector<ChatCommand> dcVipModifyCommandTable =
        {
            { "points", rbac::RBAC_PERM_COMMAND_DC_VIP_MODIFY_POINTS, true,  &HandleDCVipModifyPointsCommand, "" },
        };
        static std::vector<ChatCommand> dcVipSetCommandTable =
        {
            { "vipLevel", rbac::RBAC_PERM_COMMAND_DC_VIP_SET_VIPLEVEL, true,  &HandleDCVipSetVipLevelCommand, "" },
            { "points",   rbac::RBAC_PERM_COMMAND_DC_VIP_SET_POINTS,   true,  &HandleDCVipSetPointsCommand,   "" },
        };
        static std::vector<ChatCommand> dcVipCommandTable =
        {
            { "set",    rbac::RBAC_PERM_COMMAND_DC_VIP_SET,    true,  NULL, "", dcVipSetCommandTable },
            { "modify", rbac::RBAC_PERM_COMMAND_DC_VIP_MODIFY, true,  NULL, "", dcVipModifyCommandTable },
            { "add",    rbac::RBAC_PERM_COMMAND_DC_VIP_ADD,    true,  NULL, "", dcVipAddCommandTable},
        };
        static std::vector<ChatCommand> dcReloadCommandTable =
        {
            { "system",       rbac::RBAC_PERM_COMMAND_DC_RELOAD_MODULE,       true,  &HandleDCReloadModuleCommand,       "" },
            { "strings",      rbac::RBAC_PERM_COMMAND_DC_RELOAD_STRINGS,      true,  &HandleDCReloadStringsCommand,      "" },
            { "costs",        rbac::RBAC_PERM_COMMAND_DC_RELOAD_COSTS,        true,  &HandleDCReloadCostsCommand,        "" },
            { "requirements", rbac::RBAC_PERM_COMMAND_DC_RELOAD_REQUIREMENTS, true,  &HandleDCReloadRequirementsCommand, "" },
            { "req",          rbac::RBAC_PERM_COMMAND_DC_RELOAD_REQUIREMENTS, true,  &HandleDCReloadRequirementsCommand, "" },
            { "icons",        rbac::RBAC_PERM_COMMAND_DC_RELOAD_ICONS,        true,  &HandleDCReloadIconsCommand,        "" },
            { "items",        rbac::RBAC_PERM_COMMAND_DC_RELOAD_ITEMS,        true,  &HandleDCReloadItemsCommand,        "" },
            { "config",       rbac::RBAC_PERM_COMMAND_DC_RELOAD_CONFIG,       true,  &HandleDCReloadConfigCommand,       "" },
        };
        static std::vector<ChatCommand> dcCommandTable =
        {
            { "reload", rbac::RBAC_PERM_COMMAND_DC_RELOAD, true,  NULL, "", dcReloadCommandTable },
            { "vip",    rbac::RBAC_PERM_COMMAND_DC_VIP,    true,  NULL, "", dcVipCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "dc",    rbac::RBAC_PERM_COMMAND_DC,    true,  NULL,              "", dcCommandTable },
            { "vip",   rbac::RBAC_PERM_COMMAND_VIP,   false, &HandleVipCommand, "" },
        };
        return commandTable;
    }

    static bool HandleDCReloadModuleCommand(ChatHandler* handler, char const* args)
    {
        std::string param = (char*)args;

        if (!param.empty())
        {
            if (param == "all")
            {
                sDCMgr->ReloadAllModules();
                return true;
            }

            if (!sDCMgr->ReloadModule(param))
            {
                handler->SendSysMessage(sDCMgr->BuildDCText(STRING_SYSTEM_NOT_FOUND, param.c_str()).c_str());
                handler->SetSentErrorMessage(true);

                return false;
            }

            return true;
        }

        handler->SendSysMessage(sDCMgr->BuildDCText(STRING_PARAM_CAN_NOT_BE_NULL).c_str());
        handler->SetSentErrorMessage(true);

        return false;
    }

    static bool HandleDCReloadStringsCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadDCStrings();

        return true;
    }

    static bool HandleDCReloadCostsCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadDCCosts();

        return true;
    }

    static bool HandleDCReloadRequirementsCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadDCRequirements();

        return true;
    }

    static bool HandleDCReloadIconsCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadDCIcons();

        return true;
    }

    static bool HandleDCReloadItemsCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadDCItems();

        return true;
    }

    static bool HandleDCReloadConfigCommand(ChatHandler* handler, char const* /*args*/)
    {
        sDCMgr->LoadConfig();

        return true;
    }

    static bool HandleDCVipSetVipLevelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(sDCMgr->BuildDCText(STRING_PARAM_CAN_NOT_BE_NULL).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto accountName  = std::string("");
        auto accountID    = 0u;
        auto level        = 0u;
        auto realm        = 0;
        auto arg1         = strtok((char*)args, " ");
        auto arg2         = strtok(NULL, " ");
        auto arg3         = strtok(NULL, " ");
        bool isNameGiven  = true;
        auto player       = handler->getSelectedPlayer() ? handler->getSelectedPlayer() : handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;

        if (arg2)
        {
            accountName = arg1;
            accountID   = AccountMgr::GetId(accountName);
            level       = atoi(arg2);
            realm       = arg3 ? atoi(arg3) : ::realm.Id.Realm;
        }
        else
        {
            if (arg2)
            {
                accountName = arg1;
                accountID   = AccountMgr::GetId(accountName);
                if (!accountID)
                {
                    if (!handler->GetSession() || !player)
                    {
                        handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }

                    accountID = player->GetSession()->GetAccountId();
                    AccountMgr::GetName(accountID, accountName);
                    level = atoi(arg1);
                    realm = atoi(arg2);
                }
                else
                {
                    level = atoi(arg2);
                    realm = ::realm.Id.Realm;
                }
            }
            else
            {
                accountID = player->GetSession()->GetAccountId();
                AccountMgr::GetName(accountID, accountName);
                level = atoi(arg1);
                realm = ::realm.Id.Realm;
            }
        }

        if (!accountID)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (level < 0)
        {
            handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_VIPLEVEL_CAN_NOT_LESS_THAN_ZERO).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (realm < -1)
        {
            handler->SendSysMessage(LANG_INVALID_REALMID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sDCVIPModule->ChangeAccountVipLevel(accountID, level, realm);

        if (player && player->GetSession()->GetAccountId() == accountID)
            player->SetVipLevel(level);

        handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_YOU_CHANGE_VIPLEVEL, accountName.c_str(), level).c_str());
        return true;
    }

    static bool HandleDCVipSetPointsCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(sDCMgr->BuildDCText(STRING_PARAM_CAN_NOT_BE_NULL).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto accountName = std::string("");
        auto accountID   = 0u;
        auto points      = 0u;
        auto realm       = 0;
        auto arg1        = strtok((char*)args, " ");
        auto arg2        = strtok(NULL, " ");
        auto arg3        = strtok(NULL, " ");
        bool isNameGiven = true;
        auto player      = handler->getSelectedPlayer() ? handler->getSelectedPlayer() : handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;

        if (arg3)
        {
            accountName = arg1;
            accountID   = AccountMgr::GetId(accountName);
            points      = atoi(arg2);
            realm       = atoi(arg3);
        }
        else
        {
            if (arg2)
            {
                accountName = arg1;
                accountID   = AccountMgr::GetId(accountName);
                if (!accountID)
                {
                    if (!handler->GetSession() || !player)
                    {
                        handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }

                    accountID = player->GetSession()->GetAccountId();
                    AccountMgr::GetName(accountID, accountName);
                    points = atoi(arg1);
                    realm  = atoi(arg2);
                }
                else
                {
                    points = atoi(arg2);
                    realm = ::realm.Id.Realm;
                }
            }
            else
            {
                accountID = player->GetSession()->GetAccountId();
                AccountMgr::GetName(accountID, accountName);
                points = atoi(arg1);
                realm  = ::realm.Id.Realm;
            }
        }

        if (!accountID)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (points < 0)
        {
            handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_POINTS_CAN_NOT_LESS_THAN_ZERO).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (realm < -1)
        {
            handler->SendSysMessage(LANG_INVALID_REALMID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sDCVIPModule->ChangeAccountPoints(accountID, points, realm);

        if (player && player->GetSession()->GetAccountId() == accountID)
            player->SetPoints(points);

        handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_YOU_CHANGE_POINTS, accountName.c_str(), points).c_str());
        return true;
    }

    static bool HandleDCVipModifyPointsCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(sDCMgr->BuildDCText(STRING_PARAM_CAN_NOT_BE_NULL).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto accountName = std::string("");
        auto accountID   = 0u;
        auto points      = 0;
        auto realm       = 0;
        auto arg1        = strtok((char*)args, " ");
        auto arg2        = strtok(NULL, " ");
        auto arg3        = strtok(NULL, " ");
        bool isNameGiven = true;
        auto player      = handler->getSelectedPlayer() ? handler->getSelectedPlayer() : handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;

        if (arg3)
        {
            accountName = arg1;
            accountID   = AccountMgr::GetId(accountName);
            points      = atoi(arg2);
            realm       = atoi(arg3);
        }
        else
        {
            if (arg2)
            {
                accountName = arg1;
                accountID   = AccountMgr::GetId(accountName);
                if (!accountID)
                {
                    if (!handler->GetSession() || !player)
                    {
                        handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }

                    accountID = player->GetSession()->GetAccountId();
                    AccountMgr::GetName(accountID, accountName);
                    points = atoi(arg1);
                    realm  = atoi(arg2);
                }
                else
                {
                    points = atoi(arg2);
                    realm = ::realm.Id.Realm;
                }
            }
            else
            {
                accountID = player->GetSession()->GetAccountId();
                AccountMgr::GetName(accountID, accountName);
                points = atoi(arg1);
                realm = ::realm.Id.Realm;
            }
        }

        if (!accountID)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (realm < -1)
        {
            handler->SendSysMessage(LANG_INVALID_REALMID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sDCVIPModule->ModifyAccountPoints(accountID, points, realm);

        if (player && player->GetSession()->GetAccountId() == accountID)
            player->ModifyPoints(points);

        handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_YOU_ADD_POINTS, points, accountName.c_str()).c_str());
        return true;
    }

    static bool HandleDCVipAddVendorCommand(ChatHandler* handler, char const* args)
    {
        auto target = handler->GetSession() ? handler->getSelectedCreature() : nullptr;

        if (!*args && !target)
        {
            if (handler->GetSession())
                handler->SendSysMessage(LANG_SELECT_CREATURE);
            else
                handler->SendSysMessage(sDCMgr->BuildDCText(STRING_PARAM_CAN_NOT_BE_NULL).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto entry = *args ? atoi(args) : target->GetEntry();
        auto creature = sObjectMgr->GetCreatureTemplate(entry);
        if (!creature || !(creature->npcflag & UNIT_NPC_FLAG_VENDOR))
        {
            handler->SendSysMessage(sDCMgr->BuildDCText(STRING_ENTRY_IS_NOT_A_VENDOR, entry).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        sDCVIPModule->AddVipVendor(entry);

        WorldDatabase.PExecute("INSERT INTO dc_vip_vendor (entry) VALUES (%u)", entry);

        handler->SendSysMessage(sDCMgr->BuildDCText(STRING_ADD_VIP_VENDOR_SUCCESS, entry).c_str());

        return true;
    }

    static bool HandleVipCommand(ChatHandler* handler, char const* args)
    {
        if (!handler->GetSession())
        {
            handler->SendSysMessage(LANG_CMD_SYNTAX);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto player = handler->GetSession()->GetPlayer();
        if (handler->GetSession()->GetSecurity() == SEC_ADMINISTRATOR && handler->getSelectedPlayer())
            player = handler->getSelectedPlayer();

        handler->PSendSysMessage(sDCMgr->BuildDCText(STRING_VIPLEVEL_AND_POINTS, player->GetVipLevel(), player->GetPoints()).c_str());
        return true;
    }
};

void AddSC_DC_commandscript()
{
    new DCCommand();
}