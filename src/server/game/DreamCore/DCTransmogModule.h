#ifndef _DC_TRANSMOG_MODULE_H_
#define _DC_TRANSMOG_MODULE_H_

#define MAX_PRESETS_LIMIT 25

#include "DCMgr.h"

class Item;
class Player;
class WorldSession;
struct ItemTemplate;

class DCTransmogModule : public DCModule
{
public:
    static DCTransmogModule* instance();

    void Reload() override;

    bool EnableSetInfo;
    uint32 SetNpcText;

    bool EnableSets;
    uint8 MaxSets;
    float SetCostModifier;
    int32 SetCopperCost;

    void PresetTransmog(Player* player, Item* itemTransmogrified, uint32 fakeEntry, uint8 slot);

    bool EnableTransmogInfo;
    uint32 TransmogNpcText;

    // Use IsAllowed() and IsNotAllowed()
    // these are thread unsafe, but assumed to be static data so it should be safe
    std::set<uint32> Allowed;
    std::set<uint32> NotAllowed;

    float ScaledCostModifier;
    int32 CopperCost;

    bool RequireToken;
    uint32 TokenEntry;
    uint32 TokenAmount;

    bool AllowPoor;
    bool AllowCommon;
    bool AllowUncommon;
    bool AllowRare;
    bool AllowEpic;
    bool AllowLegendary;
    bool AllowArtifact;
    bool AllowHeirloom;

    bool AllowMixedArmorTypes;
    bool AllowMixedWeaponTypes;
    bool AllowFishingPoles;

    bool IgnoreReqRace;
    bool IgnoreReqClass;
    bool IgnoreReqSkill;
    bool IgnoreReqSpell;
    bool IgnoreReqLevel;
    bool IgnoreReqEvent;
    bool IgnoreReqStats;

    bool IsAllowed(uint32 entry) const;
    bool IsNotAllowed(uint32 entry) const;
    bool IsAllowedQuality(uint32 quality) const;
    bool IsRangedWeapon(uint32 Class, uint32 SubClass) const;

    void LoadConfig(bool reload); // thread unsafe

    std::string GetItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y) const;
    std::string GetSlotIcon(uint8 slot, uint32 width, uint32 height, int x, int y) const;
    const char * GetSlotName(uint8 slot, WorldSession* session) const;
    std::string GetItemLink(Item* item, WorldSession* session) const;
    std::string GetItemLink(uint32 entry, WorldSession* session) const;
    void UpdateItem(Player* player, Item* item) const;
    void DeleteFakeEntry(Player* player, Item* item);
    void SetFakeEntry(Player* player, Item* item, uint32 entry);

    DCStringDef Transmogrify(Player* player, ObjectGuid itemGUID, uint8 slot, bool no_cost = false);
    bool CanTransmogrifyItemWithItem(Player* player, ItemTemplate const* destination, ItemTemplate const* source) const;
    bool SuitableForTransmogrification(Player* player, ItemTemplate const* proto) const;
    uint32 GetSpecialPrice(ItemTemplate const* proto) const;
    std::vector<ObjectGuid> GetItemList(const Player* player) const;

protected:
    void Initialize()     override;
    void LoadDatabase()   override;

private:
    DCTransmogModule();
};

#define sDCTransmogModule DCTransmogModule::instance()

#endif
