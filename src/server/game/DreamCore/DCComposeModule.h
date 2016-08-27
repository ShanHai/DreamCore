#ifndef _DC_COMPOSE_SYSTEM_H_
#define _DC_COMPOSE_SYSTEM_H_

#include "DCMgr.h"

struct ComposeLoot
{
    uint32          id;
    uint32          item;
    float           chance;
    uint8           lootGroup;
    uint8           minCount;
    uint8           maxCount;
    uint32          reqID;
};

struct ComposeCategory
{
    uint16          entry;
    uint32          textID;
    uint32          costID;
    uint32          reqID;
};

struct ComposeDefinition
{
    uint16          category;
    uint32          id;
    uint8           iconID;
    std::string     iconExtend;
    std::string     text;
    uint32          lootID;
    uint32          costID;
    uint32          reqID;
    uint8           lootCountLimit;
    bool            useCategorySettings;
    uint16          subCategory;
};

typedef std::multimap<uint32, ComposeLoot>                ComposeLootContainer;
typedef std::unordered_map<uint16, ComposeCategory>       ComposeCategoryContainer;
typedef std::multimap<uint16, ComposeDefinition>          ComposeDefinitionContainer;
typedef std::pair<uint32, ComposeLoot>                    ComposeLootPair;
typedef std::pair<uint16, ComposeDefinition>              ComposeDefinitionPair;

class DCComposeModule : public DCModule
{
    public:
        static DCComposeModule* instance();

        void ReloadDataBase() override;

        auto GetComposeLootMapBounds(uint32 id) const
        {
            return m_composeLoot.equal_range(id);
        }

        auto GetComposeDefinitionsMapBounds(uint16 category) const
        {
            return m_composeDefinitions.equal_range(category);
        }

        const ComposeLoot* GetComposeLoot(uint32 id, uint32 item)
        {
            auto bounds = GetComposeLootMapBounds(id);
            if (bounds.first == bounds.second)
                return nullptr;

            auto itr = std::find_if(bounds.first, bounds.second, [&](const auto& pair) { return pair.second.item == item; });
            return (itr == bounds.second ? nullptr : &itr->second);
        }

        const ComposeCategory* GetComposeCategory(uint16 entry)
        {
            auto itr = m_composeCategories.find(entry);
            return (itr == m_composeCategories.end() ? nullptr : &itr->second);
        }

        const ComposeDefinition* GetComposeDefinition(uint16 category, uint32 id)
        {
            auto bounds = GetComposeDefinitionsMapBounds(category);
            if (bounds.first == bounds.second)
                return nullptr;

            auto itr = std::find_if(bounds.first, bounds.second, [&](const auto& pair) { return pair.second.id == id; });
            return (itr == bounds.second ? nullptr : &itr->second);
        }

    protected:
        void Initialize()     override;
        void LoadDatabase()   override;

        void LoadComposeLoot();
        void LoadComposeCategories();
        void LoadComposeDefinitions();

    private:
        DCComposeModule();

        ComposeLootContainer          m_composeLoot;
        ComposeCategoryContainer      m_composeCategories;
        ComposeDefinitionContainer    m_composeDefinitions;
};

#define sDCComposeModule DCComposeModule::instance()

#endif