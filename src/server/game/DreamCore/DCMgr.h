#ifndef _DC_MGR_H_
#define _DC_MGR_H_

#include "Common.h"
#include "DCStringDef.h"
#include "DCConfigDef.h"

class DCMgr;
class DCModule;
class DCTeleportModule;
class DCComposeModule;
class DCVIPModule;
class DCTransmogModule;

enum DCItemType
{
    DC_ITEM_TYPE_SCROLL_XP          = 0,
    DC_ITEM_TYPE_SCROLL_PROFESSION  = 1,
    DC_ITEM_TYPE_SCROLL_TELEPORT    = 2,
    DC_ITEM_TYPE_SCROLL_UNBINDINST  = 3,

    DC_ITEM_TYPE_COUNT
};

struct DCCost
{
    uint32  entry;
    uint32  money;
    uint32  honor;
    uint32  arenaPoint;
    uint32  point;
    uint32  items[10];
    uint32  itemsCount[10];
    uint8   lastItemIndex;
};

struct DCRequirement
{
    uint32  entry;
    uint8   faction;
    uint16  vip;
    uint8   level;
};

struct DCString
{
    StringVector strings;
};

struct DCIcon
{
    uint32       displayID;
    std::string  displayName;
};

struct DCItem
{
    uint32         entry;
    DCItemType     type;
    int32          data0;
    int32          data1;
    int32          data2;
    int32          data3;
    int32          data4;
};

typedef std::unordered_map<uint32, DCString>      DCStrings;
typedef std::unordered_map<uint32, DCCost>        DCCosts;
typedef std::unordered_map<uint32, DCRequirement> DCRequirements;
typedef std::unordered_map<uint32, DCIcon>        DCIcons;
typedef std::unordered_map<uint32, DCItem>        DCItems;

class DCModule
{
    public:
        virtual void ReloadDataBase() { }

        const std::string& getName() { return m_name; }
        bool enabled() { return m_enabled; }
        void enabled(bool e) { m_enabled = e; }

    protected:
        DCModule(const char* name);

        virtual void Initialize() = 0;
        virtual void LoadDatabase() { };

    private:
        bool        m_enabled = true;
        std::string m_name;
};

class DCMgr
{
    friend class DCModule;

    public:
        static DCMgr* instance();

        void Initialize();
        void LoadConfig();
        void LoadDCStrings();
        void LoadDCCosts();
        void LoadDCRequirements();
        void LoadDCIcons();
        void LoadDCItems();

        bool ReloadModule(const std::string& name);
        void ReloadAllModules();

        // Set a configuration element (see #DCConfigBool)
        void setConfigBool(DCConfigBool index, bool value)
        {
            if (index < DC_CONFIG_BOOL_COUNT)
                m_config_bool[index] = value;
        }

        // Get a configuration element (see #DCConfigBool)
        bool getConfigBool(DCConfigBool index) const
        {
            return index < DC_CONFIG_BOOL_COUNT ? m_config_bool[index] : 0;
        }

        // Set a configuration element (see #DCConfigFloat)
        void setConfigFloat(DCConfigFloat index, float value)
        {
            if (index < DC_CONFIG_FLOAT_COUNT)
                m_config_float[index] = value;
        }

        // Get a configuration element (see #DCConfigFloat)
        float getConfigFloat(DCConfigFloat index) const
        {
            return index < DC_CONFIG_FLOAT_COUNT ? m_config_float[index] : 0;
        }

        // Set a configuration element (see #DCConfigInt)
        void setConfigInt(DCConfigInt index, uint32 value)
        {
            if (index < DC_CONFIG_INT_COUNT)
                m_config_int[index] = value;
        }

        // Get a configuration element (see #DCConfigInt)
        uint32 getConfigInt(DCConfigInt index) const
        {
            return index < DC_CONFIG_INT_COUNT ? m_config_int[index] : 0;
        }

        DCString const* GetDCString(uint32 entry) const
        {
            const auto itr = m_DCStrings.find(entry);
            return itr == m_DCStrings.end() ? nullptr : &itr->second;
        }

        DCCost const* GetDCCost(uint32 entry) const
        {
            const auto itr = m_DCCosts.find(entry);
            return itr == m_DCCosts.end() ? nullptr : &itr->second;
        }

        DCRequirement const* GetDCRequirement(uint32 entry) const
        {
            const auto itr = m_DCRequirements.find(entry);
            return itr == m_DCRequirements.end() ? nullptr : &itr->second;
        }

        DCIcon const* GetDCIcon(uint32 displayID) const
        {
            const auto itr = m_DCIcons.find(displayID);
            return itr == m_DCIcons.end() ? nullptr : &itr->second;
        }

        const char* GetDCIconName(uint32 displayID) const
        {
            const auto icon = GetDCIcon(displayID);
            return icon ? icon->displayName.c_str() : "";
        }

        DCItem const* GetDCItem(uint32 entry)
        {
            const auto itr = m_DCItems.find(entry);
            return itr == m_DCItems.end() ? nullptr : &itr->second;
        }

        const char* GetDCString(uint32 entry, LocaleConstant locale) const;
        std::string BuildDCText(uint32 entry, ...);
        std::string BuildMoneyText(uint32 money, bool tag = false);
        std::string BuildFullCostText(uint32 costID, bool icon = false, bool prefix = true, LocaleConstant locale = LOCALE_zhCN);
        std::string BuildFullCostText(const DCCost* cost, bool icon = false, bool prefix = true, LocaleConstant locale = LOCALE_zhCN);
        std::string BuildCostText(uint32 costID, bool buildGold = true, bool prefix = true);
        std::string BuildCostText(const DCCost* cost, bool buildGold = true, bool prefix = true);
        std::string BuildCostText(uint32 money = 0, uint32 honor = 0, uint32 arenaPoint = 0, uint32 point = 0, bool buildGold = true, bool prefix = true);

        bool CheckRequirement(const Player* player, const DCRequirement* req);
        bool CheckRequirement(const Player* player, uint32 reqID);
        bool CheckCost(const Player* player, const DCCost* cost);
        bool CheckCost(const Player* player, uint32 costID);
        bool DoCost(Player* player, uint32 costID);
        bool DoCost(Player* player, const DCCost* cost);

        DCModule* GetModule(const std::string& name);

    private:
        DCMgr() { }
        virtual ~DCMgr() { }

        void AddModule(DCModule *const newModule);

    private:
        uint32    m_config_int[DC_CONFIG_INT_COUNT];
        bool      m_config_bool[DC_CONFIG_BOOL_COUNT];
        float     m_config_float[DC_CONFIG_FLOAT_COUNT];

        DCStrings      m_DCStrings;
        DCCosts        m_DCCosts;
        DCRequirements m_DCRequirements;
        DCIcons        m_DCIcons;
        DCItems        m_DCItems;

        std::vector<DCModule*> m_modules;
};

#define sDCMgr DCMgr::instance()

#endif
