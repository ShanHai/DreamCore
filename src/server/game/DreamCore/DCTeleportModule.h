#ifndef _DC_TELEPORT_SYSTEM_H_
#define _DC_TELEPORT_SYSTEM_H_

#include "DCMgr.h"

struct TeleportDefinition
{
    uint32          category;
    uint32          id;
    uint8           iconID;
    std::string     iconExtend;
    std::string     text;
    bool            customText;
    uint32          teleID;
    uint32          subCategory;
    std::string     boxText;
    uint32          costID;
    uint32          reqID;
};

struct TeleportCategory
{
    uint32          entry;
    uint32          textID;
};

typedef std::unordered_map<uint32, TeleportCategory>   TeleportCategoryContainer;
typedef std::multimap<uint32, TeleportDefinition>      TeleportDefinitionContainer;

class DCTeleportModule : public DCModule
{
    public:
        static DCTeleportModule* instance();

        void Reload() override;

        auto GetTeleportDefinitionsMapBounds(uint32 category) const
        {
            return m_teleportDefinitions.equal_range(category);
        }

        const TeleportCategory* GetTeleportCategory(uint32 entry)
        {
            auto itr = std::find_if(m_teleportCategories.begin(), m_teleportCategories.end(), [&](const auto& pair) { return pair.second.entry == entry; });
            return (itr == m_teleportCategories.end() ? nullptr : &itr->second);
        }

        const TeleportDefinition* GetTeleportDefinition(uint32 category, uint32 id)
        {
            auto bounds = GetTeleportDefinitionsMapBounds(category);
            if (bounds.first == bounds.second)
                return nullptr;

            auto itr = std::find_if(bounds.first, bounds.second, [&](const auto& pair) { return pair.second.id == id; });
            return (itr == bounds.second ? nullptr : &itr->second);
        }

    protected:
        void Initialize()     override;
        void LoadDatabase()   override;

        void LoadTeleportCategories();
        void LoadTeleportDefinitions();

    private:
        DCTeleportModule();

        TeleportCategoryContainer    m_teleportCategories;
        TeleportDefinitionContainer  m_teleportDefinitions;
};

#define sDCTeleportModule DCTeleportModule::instance()

#endif