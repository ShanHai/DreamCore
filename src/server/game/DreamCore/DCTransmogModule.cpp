#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "DCConfig.h"
#include "DCTransmogModule.h"

DCTransmogModule * DCTransmogModule::instance()
{
    static DCTransmogModule instance;
    return &instance;
}

DCTransmogModule::DCTransmogModule() : DCModule("transmog")
{
    TC_LOG_INFO("dc.transmog", "Initializing DCTransmogModule...");

    Initialize();

    TC_LOG_INFO("dc.transmog", "DCTransmogModule initialized.");
}

void DCTransmogModule::Reload()
{
    TC_LOG_INFO("dc.transmog", "Reloading DCTransmogModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.transmog", "DCTransmogModule reload complete...");

    LoadConfig(true);
}

void DCTransmogModule::Initialize()
{
    LoadDatabase();
    LoadConfig(false);
}

void DCTransmogModule::LoadDatabase()
{
}

void DCTransmogModule::PresetTransmog(Player* player, Item* itemTransmogrified, uint32 fakeEntry, uint8 slot)
{
    TC_LOG_DEBUG("dc.transmog", "Transmogrification::PresetTransmog");

    if (!EnableSets || !player || !itemTransmogrified || slot >= EQUIPMENT_SLOT_END)
        return;

    if (!CanTransmogrifyItemWithItem(player, itemTransmogrified->GetTemplate(), sObjectMgr->GetItemTemplate(fakeEntry)))
        return;

    SetFakeEntry(player, itemTransmogrified, fakeEntry);

    itemTransmogrified->UpdatePlayedTime(player);

    itemTransmogrified->SetOwnerGUID(player->GetGUID());
    itemTransmogrified->SetNotRefundable(player);
    itemTransmogrified->ClearSoulboundTradeable(player);
}

const char* DCTransmogModule::GetSlotName(uint8 slot, WorldSession* session) const
{
    if (!session)
        return nullptr;

    switch (slot)
    {
        case EQUIPMENT_SLOT_HEAD:            return  session->GetDCString(STRING_EQUIPMENT_SLOT_HEAD);
        case EQUIPMENT_SLOT_SHOULDERS:       return  session->GetDCString(STRING_EQUIPMENT_SLOT_SHOULDERS);
        case EQUIPMENT_SLOT_BODY:            return  session->GetDCString(STRING_EQUIPMENT_SLOT_SHIRT);
        case EQUIPMENT_SLOT_CHEST:           return  session->GetDCString(STRING_EQUIPMENT_SLOT_CHEST);
        case EQUIPMENT_SLOT_WAIST:           return  session->GetDCString(STRING_EQUIPMENT_SLOT_WAIST);
        case EQUIPMENT_SLOT_LEGS:            return  session->GetDCString(STRING_EQUIPMENT_SLOT_LEGS);
        case EQUIPMENT_SLOT_FEET:            return  session->GetDCString(STRING_EQUIPMENT_SLOT_FEET);
        case EQUIPMENT_SLOT_WRISTS:          return  session->GetDCString(STRING_EQUIPMENT_SLOT_WRISTS);
        case EQUIPMENT_SLOT_HANDS:           return  session->GetDCString(STRING_EQUIPMENT_SLOT_HANDS);
        case EQUIPMENT_SLOT_BACK:            return  session->GetDCString(STRING_EQUIPMENT_SLOT_BACK);
        case EQUIPMENT_SLOT_MAINHAND:        return  session->GetDCString(STRING_EQUIPMENT_SLOT_MAINHAND);
        case EQUIPMENT_SLOT_OFFHAND:         return  session->GetDCString(STRING_EQUIPMENT_SLOT_OFFHAND);
        case EQUIPMENT_SLOT_RANGED:          return  session->GetDCString(STRING_EQUIPMENT_SLOT_RANGED);
        case EQUIPMENT_SLOT_TABARD:          return  session->GetDCString(STRING_EQUIPMENT_SLOT_TABARD);
        default: return nullptr;
    }
}

std::string DCTransmogModule::GetItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y) const
{
    std::ostringstream ss;
    ss << "|TInterface";
    const ItemDisplayInfoEntry* dispInfo = nullptr;
    if (auto temp = sObjectMgr->GetItemTemplate(entry))
    {
        dispInfo = sItemDisplayInfoStore.LookupEntry(temp->DisplayInfoID);
        if (dispInfo)
            ss << "/ICONS/" << dispInfo->inventoryIcon;
    }
    if (!dispInfo)
        ss << "/InventoryItems/WoWUnknownItem01";
    ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
    return ss.str();
}

std::string DCTransmogModule::GetSlotIcon(uint8 slot, uint32 width, uint32 height, int x, int y) const
{
    std::ostringstream ss;
    ss << "|TInterface/PaperDoll/";

    switch (slot)
    {
        case EQUIPMENT_SLOT_HEAD:      ss << "UI-PaperDoll-Slot-Head";          break;
        case EQUIPMENT_SLOT_SHOULDERS: ss << "UI-PaperDoll-Slot-Shoulder";      break;
        case EQUIPMENT_SLOT_BODY:      ss << "UI-PaperDoll-Slot-Shirt";         break;
        case EQUIPMENT_SLOT_CHEST:     ss << "UI-PaperDoll-Slot-Chest";         break;
        case EQUIPMENT_SLOT_WAIST:     ss << "UI-PaperDoll-Slot-Waist";         break;
        case EQUIPMENT_SLOT_LEGS:      ss << "UI-PaperDoll-Slot-Legs";          break;
        case EQUIPMENT_SLOT_FEET:      ss << "UI-PaperDoll-Slot-Feet";          break;
        case EQUIPMENT_SLOT_WRISTS:    ss << "UI-PaperDoll-Slot-Wrists";        break;
        case EQUIPMENT_SLOT_HANDS:     ss << "UI-PaperDoll-Slot-Hands";         break;
        case EQUIPMENT_SLOT_BACK:      ss << "UI-PaperDoll-Slot-Chest";         break;
        case EQUIPMENT_SLOT_MAINHAND:  ss << "UI-PaperDoll-Slot-MainHand";      break;
        case EQUIPMENT_SLOT_OFFHAND:   ss << "UI-PaperDoll-Slot-SecondaryHand"; break;
        case EQUIPMENT_SLOT_RANGED:    ss << "UI-PaperDoll-Slot-Ranged";        break;
        case EQUIPMENT_SLOT_TABARD:    ss << "UI-PaperDoll-Slot-Tabard";        break;
        default:                       ss << "UI-Backpack-EmptySlot";
    }

    ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
    return ss.str();
}

std::string DCTransmogModule::GetItemLink(Item* item, WorldSession* session) const
{
    auto loc  = session->GetSessionDbLocaleIndex();
    auto temp = item->GetTemplate();
    auto name = temp->Name1;

    if (auto il = sObjectMgr->GetItemLocale(temp->ItemId))
        ObjectMgr::GetLocaleString(il->Name, loc, name);

    if (auto itemRandPropId = item->GetItemRandomPropertyId())
    {
        char* const* suffix = nullptr;
        if (itemRandPropId < 0)
        {
            if (auto itemRandEntry = sItemRandomSuffixStore.LookupEntry(-item->GetItemRandomPropertyId()))
                suffix = itemRandEntry->nameSuffix;
        }
        else
        {
            if (auto itemRandEntry = sItemRandomPropertiesStore.LookupEntry(item->GetItemRandomPropertyId()))
                suffix = itemRandEntry->nameSuffix;
        }

        if (suffix)
        {
            std::string test(suffix[(name != temp->Name1) ? loc : DEFAULT_LOCALE]);
            if (!test.empty())
            {
                name += ' ';
                name += test;
            }
        }
    }

    std::ostringstream oss;
    oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
        "|Hitem:" << temp->ItemId << ":" <<
        item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT)   << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT)   << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2) << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3) << ":" <<
        item->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT)  << ":" <<
        item->GetItemRandomPropertyId() << ":" << item->GetItemSuffixFactor() << ":" <<
        (uint32)item->GetOwner()->getLevel() << "|h[" << name << "]|h|r";

    return oss.str();
}

std::string DCTransmogModule::GetItemLink(uint32 entry, WorldSession* session) const
{
    auto temp = sObjectMgr->GetItemTemplate(entry);
    auto loc  = session->GetSessionDbLocaleIndex();
    auto name = temp->Name1;
    if (auto il = sObjectMgr->GetItemLocale(entry))
        ObjectMgr::GetLocaleString(il->Name, loc, name);

    std::ostringstream oss;
    oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
        "|Hitem:" << entry << ":0:0:0:0:0:0:0:0:0|h[" << name << "]|h|r";

    return oss.str();
}

void DCTransmogModule::UpdateItem(Player* player, Item* item) const
{
    if (item->IsEquipped())
    {
        player->SetVisibleItemSlot(item->GetSlot(), item);
        if (player->IsInWorld())
            item->SendUpdateToPlayer(player);
    }
}

void DCTransmogModule::SetFakeEntry(Player* player, Item* item, uint32 entry)
{
    item->SetFakeEntry(entry);

    UpdateItem(player, item);
}

void DCTransmogModule::DeleteFakeEntry(Player* player, Item *item)
{
    item->SetFakeEntry(0);

    UpdateItem(player, item);
}

DCStringDef DCTransmogModule::Transmogrify(Player* player, ObjectGuid itemGUID, uint8 slot, bool no_cost)
{
    if (slot >= EQUIPMENT_SLOT_END)
    {
        TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::Transmogrify - Player (GUID: %u, name: %s) tried to transmogrify an item (guid: %u) with a wrong slot (%u) when transmogrifying items.", player->GetGUID().GetCounter(), player->GetName().c_str(), itemGUID.GetCounter(), slot);
        return STRING_TRANSMOG_INVALID_SLOT;
    }

    Item* itemTransmogrifier = nullptr;
    if (itemGUID)
    {
        itemTransmogrifier = player->GetItemByGuid(itemGUID);
        if (!itemTransmogrifier)
        {
            TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::Transmogrify - Player (GUID: %u, name: %s) tried to transmogrify with an invalid item (GUID: %u).", player->GetGUID().GetCounter(), player->GetName().c_str(), itemGUID.GetCounter());
            return STRING_TRANSMOG_MISSING_SRC_ITEM;
        }
    }

    auto itemTransmogrified = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!itemTransmogrified)
    {
        TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::Transmogrify - Player (GUID: %u, name: %s) tried to transmogrify an invalid item in a valid slot (slot: %u).", player->GetGUID().GetCounter(), player->GetName().c_str(), slot);
        return STRING_TRANSMOG_MISSING_DEST_ITEM;
    }

    if (!itemTransmogrifier)
    {
        DeleteFakeEntry(player, itemTransmogrified);
    }
    else
    {
        if (!CanTransmogrifyItemWithItem(player, itemTransmogrified->GetTemplate(), itemTransmogrifier->GetTemplate()))
        {
            TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::Transmogrify - Player (GUID: %u, name: %s) failed CanTransmogrifyItemWithItem (%u with %u).", player->GetGUID().GetCounter(), player->GetName().c_str(), itemTransmogrified->GetEntry(), itemTransmogrifier->GetEntry());
            return STRING_TRANSMOG_INVALID_ITEMS;
        }

        if (!no_cost)
        {
            if (RequireToken)
            {
                if (player->HasItemCount(TokenEntry, TokenAmount))
                    player->DestroyItemCount(TokenEntry, TokenAmount, true);
                else
                    return STRING_TRANSMOG_NOT_ENOUGH_TOKENS;
            }

            int32 cost = 0;
            cost = GetSpecialPrice(itemTransmogrified->GetTemplate());
            cost *= ScaledCostModifier;
            cost += CopperCost;

            if (cost)
            {
                if (cost < 0)
                    TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::Transmogrify - Player (GUID: %u, name: %s) transmogrification invalid cost (non negative, amount %i). Transmogrified %u with %u", player->GetGUID().GetCounter(), player->GetName().c_str(), -cost, itemTransmogrified->GetEntry(), itemTransmogrifier->GetEntry());
                else
                {
                    if (!player->HasEnoughMoney(cost))
                        return STRING_TRANSMOG_NOT_ENOUGH_MONEY;
                    player->ModifyMoney(-cost, false);
                }
            }
        }

        SetFakeEntry(player, itemTransmogrified, itemTransmogrifier->GetEntry());

        itemTransmogrified->UpdatePlayedTime(player);

        itemTransmogrified->SetOwnerGUID(player->GetGUID());
        itemTransmogrified->SetNotRefundable(player);
        itemTransmogrified->ClearSoulboundTradeable(player);

        if (itemTransmogrifier->GetTemplate()->Bonding == BIND_WHEN_EQUIPED || itemTransmogrifier->GetTemplate()->Bonding == BIND_WHEN_USE)
            itemTransmogrifier->SetBinding(true);

        itemTransmogrifier->SetOwnerGUID(player->GetGUID());
        itemTransmogrifier->SetNotRefundable(player);
        itemTransmogrifier->ClearSoulboundTradeable(player);
    }

    return STRING_TRANSMOG_SUCCESS;
}

bool DCTransmogModule::CanTransmogrifyItemWithItem(Player* player, ItemTemplate const* target, ItemTemplate const* source) const
{
    TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::CanTransmogrifyItemWithItem");

    if (!target || !source)
        return false;

    if (source->ItemId == target->ItemId)
        return false;

    if (!SuitableForTransmogrification(player, target) || !SuitableForTransmogrification(player, source)) // if (!transmogrified->CanTransmogrify() || !transmogrifier->CanBeTransmogrified())
        return false;

    if (source->InventoryType == INVTYPE_BAG ||
        source->InventoryType == INVTYPE_RELIC ||
        // source->InventoryType == INVTYPE_BODY ||
        source->InventoryType == INVTYPE_FINGER ||
        source->InventoryType == INVTYPE_TRINKET ||
        source->InventoryType == INVTYPE_AMMO ||
        source->InventoryType == INVTYPE_QUIVER)
        return false;

    if (!SuitableForTransmogrification(player, target) || !SuitableForTransmogrification(player, source)) // if (!transmogrified->CanTransmogrify() || !transmogrifier->CanBeTransmogrified())
        return false;

    if (IsRangedWeapon(source->Class, source->SubClass) != IsRangedWeapon(target->Class, target->SubClass))
        return false;

    if (source->SubClass != target->SubClass && !IsRangedWeapon(target->Class, target->SubClass))
    {
        if (source->Class == ITEM_CLASS_ARMOR && !AllowMixedArmorTypes)
            return false;
        if (source->Class == ITEM_CLASS_WEAPON && !AllowMixedWeaponTypes)
            return false;
    }

    if (source->InventoryType != target->InventoryType)
    {
        if (source->Class == ITEM_CLASS_WEAPON && !((IsRangedWeapon(target->Class, target->SubClass) ||
            ((target->InventoryType == INVTYPE_WEAPON || target->InventoryType == INVTYPE_2HWEAPON) &&
            (source->InventoryType == INVTYPE_WEAPON || source->InventoryType == INVTYPE_2HWEAPON)) ||
                ((target->InventoryType == INVTYPE_WEAPONMAINHAND || target->InventoryType == INVTYPE_WEAPONOFFHAND) &&
            (source->InventoryType == INVTYPE_WEAPON || source->InventoryType == INVTYPE_2HWEAPON)))))
            return false;
        if (source->Class == ITEM_CLASS_ARMOR &&
            !((source->InventoryType == INVTYPE_CHEST || source->InventoryType == INVTYPE_ROBE) &&
            (target->InventoryType == INVTYPE_CHEST || target->InventoryType == INVTYPE_ROBE)))
            return false;
    }

    return true;
}

bool DCTransmogModule::SuitableForTransmogrification(Player* player, ItemTemplate const* proto) const
{
    if (proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON)
        return false;

    if (IsAllowed(proto->ItemId))
        return true;

    if (IsNotAllowed(proto->ItemId))
        return false;

    if (!AllowFishingPoles && proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        return false;

    if (!IsAllowedQuality(proto->Quality))
        return false;

    if ((proto->Flags2 & ITEM_FLAGS_EXTRA_HORDE_ONLY) && player->GetTeam() != HORDE)
        return false;

    if ((proto->Flags2 & ITEM_FLAGS_EXTRA_ALLIANCE_ONLY) && player->GetTeam() != ALLIANCE)
        return false;

    if (!IgnoreReqClass && (proto->AllowableClass & player->getClassMask()) == 0)
        return false;

    if (!IgnoreReqRace && (proto->AllowableRace & player->getRaceMask()) == 0)
        return false;

    if (!IgnoreReqSkill && proto->RequiredSkill != 0)
    {
        if (player->GetSkillValue(proto->RequiredSkill) == 0)
            return false;
        else if (player->GetSkillValue(proto->RequiredSkill) < proto->RequiredSkillRank)
            return false;
    }

    if (!IgnoreReqSpell && proto->RequiredSpell != 0 && !player->HasSpell(proto->RequiredSpell))
        return false;

    if (!IgnoreReqLevel && player->getLevel() < proto->RequiredLevel)
        return false;

    if (!IgnoreReqEvent && proto->HolidayId && !IsHolidayActive((HolidayIds)proto->HolidayId))
        return false;

    if (!IgnoreReqStats && !proto->RandomProperty && !proto->RandomSuffix)
    {
        for (uint8 i = 0; i < proto->StatsCount; ++i)
            if (proto->ItemStat[i].ItemStatValue != 0)
                return true;
        return false;
    }

    return true;
}

uint32 DCTransmogModule::GetSpecialPrice(ItemTemplate const* proto) const
{
    return proto->SellPrice < 10000 ? 10000 : proto->SellPrice;
}

bool DCTransmogModule::IsRangedWeapon(uint32 Class, uint32 SubClass) const
{
    return Class == ITEM_CLASS_WEAPON && (
        SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
        SubClass == ITEM_SUBCLASS_WEAPON_GUN ||
        SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW);
}

bool DCTransmogModule::IsAllowed(uint32 entry) const
{
    return Allowed.find(entry) != Allowed.end();
}

bool DCTransmogModule::IsNotAllowed(uint32 entry) const
{
    return NotAllowed.find(entry) != NotAllowed.end();
}

bool DCTransmogModule::IsAllowedQuality(uint32 quality) const
{
    switch (quality)
    {
        case ITEM_QUALITY_POOR:      return AllowPoor;
        case ITEM_QUALITY_NORMAL:    return AllowCommon;
        case ITEM_QUALITY_UNCOMMON:  return AllowUncommon;
        case ITEM_QUALITY_RARE:      return AllowRare;
        case ITEM_QUALITY_EPIC:      return AllowEpic;
        case ITEM_QUALITY_LEGENDARY: return AllowLegendary;
        case ITEM_QUALITY_ARTIFACT:  return AllowArtifact;
        case ITEM_QUALITY_HEIRLOOM:  return AllowHeirloom;
        default: return false;
    }
}

void DCTransmogModule::LoadConfig(bool reload)
{
    TC_LOG_DEBUG("dc.transmog", "DCTransmogModule::LoadConfig");

    EnableSetInfo   = sDCConfigMgr->GetBoolDefault("DC.Transmog.EnableSetInfo",    true);
    SetNpcText      = uint32(sDCConfigMgr->GetIntDefault("DC.Transmog.SetNpcText", 12000001));
    EnableSets      = sDCConfigMgr->GetBoolDefault("DC.Transmog.EnableSets",       true);
    MaxSets         = (uint8)sDCConfigMgr->GetIntDefault("DC.Transmog.MaxSets",    10);
    SetCostModifier = sDCConfigMgr->GetFloatDefault("DC.Transmog.SetCostModifier", 3.0f);
    SetCopperCost   = sDCConfigMgr->GetIntDefault("DC.Transmog.SetCopperCost",     0);

    if (MaxSets > MAX_PRESETS_LIMIT)
        MaxSets = MAX_PRESETS_LIMIT;

    EnableTransmogInfo = sDCConfigMgr->GetBoolDefault("DC.Transmog.EnableTransmogInfo", true);
    TransmogNpcText    = uint32(sDCConfigMgr->GetIntDefault("DC.Transmog.TransmogNpcText", 12000000));

    std::istringstream issAllowed(sDCConfigMgr->GetStringDefault("DC.Transmog.Allowed", ""));
    std::istringstream issNotAllowed(sDCConfigMgr->GetStringDefault("DC.Transmog.NotAllowed", ""));
    while (issAllowed.good())
    {
        uint32 entry;
        issAllowed >> entry;
        if (issAllowed.fail())
            break;
        Allowed.insert(entry);
    }
    while (issNotAllowed.good())
    {
        uint32 entry;
        issNotAllowed >> entry;
        if (issNotAllowed.fail())
            break;
        NotAllowed.insert(entry);
    }

    ScaledCostModifier    = sDCConfigMgr->GetFloatDefault("DC.Transmog.ScaledCostModifier",   1.0f);
    CopperCost            = sDCConfigMgr->GetIntDefault("DC.Transmog.CopperCost",             0);

    RequireToken          = sDCConfigMgr->GetBoolDefault("DC.Transmog.RequireToken",          false);
    TokenEntry            = uint32(sDCConfigMgr->GetIntDefault("DC.Transmog.TokenEntry",      37711));
    TokenAmount           = uint32(sDCConfigMgr->GetIntDefault("DC.Transmog.TokenAmount",     1));

    AllowPoor             = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowPoor",             false);
    AllowCommon           = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowCommon",           false);
    AllowUncommon         = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowUncommon",         true);
    AllowRare             = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowRare",             true);
    AllowEpic             = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowEpic",             true);
    AllowLegendary        = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowLegendary",        false);
    AllowArtifact         = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowArtifact",         false);
    AllowHeirloom         = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowHeirloom",         true);
    AllowMixedArmorTypes  = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowMixedArmorTypes",  false);
    AllowMixedWeaponTypes = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowMixedWeaponTypes", false);
    AllowFishingPoles     = sDCConfigMgr->GetBoolDefault("DC.Transmog.AllowFishingPoles",     false);

    IgnoreReqRace         = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqRace",         false);
    IgnoreReqClass        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqClass",        false);
    IgnoreReqSkill        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqSkill",        false);
    IgnoreReqSpell        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqSpell",        false);
    IgnoreReqLevel        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqLevel",        false);
    IgnoreReqEvent        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqEvent",        false);
    IgnoreReqStats        = sDCConfigMgr->GetBoolDefault("DC.Transmog.IgnoreReqStats",        false);

    if (!sObjectMgr->GetItemTemplate(TokenEntry))
    {
        TC_LOG_INFO("dc.transmog", "DC.Transmog.TokenEntry (%u) does not exist. Using default (%u).", TokenEntry, 37711);
        TokenEntry = 37711;
    }
}

std::vector<ObjectGuid> DCTransmogModule::GetItemList(const Player* player) const
{
    std::vector<ObjectGuid> itemlist;

    for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            itemlist.push_back(pItem->GetGUID());

    for (uint8 i = KEYRING_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
        if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            itemlist.push_back(pItem->GetGUID());

    for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
        if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            itemlist.push_back(pItem->GetGUID());

    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = player->GetBagByPos(i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    itemlist.push_back(pItem->GetGUID());

    for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* pBag = player->GetBagByPos(i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    itemlist.push_back(pItem->GetGUID());

    return itemlist;
}

