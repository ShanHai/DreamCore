#ifndef _DC_VIP_SYSTEM_H_
#define _DC_VIP_SYSTEM_H_

#include "DCMgr.h"

typedef std::vector<uint32> DCVipVendors;

class DCVIPModule : public DCModule
{
    public:
        static DCVIPModule* instance();

        void ReloadDataBase() override;

        void ChangeAccountVipLevel(uint32 accountID, uint32 vipLevel, int32 realmID);
        void ChangeAccountPoints(uint32 accountID, uint32 points, int32 realmID);
        void ModifyAccountPoints(uint32 accountID, int32 points, int32 realmID);

        bool isVipVendor(uint32 entry) const
        {
            auto itr = std::find(m_vipVendors.begin(), m_vipVendors.end(), entry);
            return itr != m_vipVendors.end();
        }

        void AddVipVendor(uint32 entry)
        {
            if (isVipVendor(entry))
                return;
            m_vipVendors.push_back(entry);
        }

    protected:
        void Initialize()     override;
        void LoadDatabase()   override;

        void LoadVipVendors();

    private:
        DCVIPModule();

        DCVipVendors  m_vipVendors;
};

#define sDCVIPModule DCVIPModule::instance()

#endif