#ifndef _DC_CONFIG_H_
#define _DC_CONFIG_H_

#include "Define.h"

#include <string>
#include <list>
#include <mutex>
#include <boost/property_tree/ptree.hpp>

// used for loading dreamcore configuration files(dreamcore.conf)
class TC_COMMON_API DCConfigMgr
{
    DCConfigMgr() = default;
    DCConfigMgr(DCConfigMgr const&) = delete;
    DCConfigMgr& operator=(DCConfigMgr const&) = delete;
    ~DCConfigMgr() = default;

public:
    bool Load(const std::string& file);
    bool Reload();

    static DCConfigMgr* instance();

    std::string GetStringDefault(std::string const& name, const std::string& def) const;
    bool GetBoolDefault(std::string const& name, bool def) const;
    int GetIntDefault(std::string const& name, int def) const;
    float GetFloatDefault(std::string const& name, float def) const;

    std::string const& GetFilename();
    std::list<std::string> GetKeysByString(std::string const& name);

private:
    std::string                   m_filename;
    boost::property_tree::ptree   m_config;
    std::mutex                    m_configLock;

    template<class T>
    T GetValueDefault(std::string const& name, T def) const;
};

#define sDCConfigMgr DCConfigMgr::instance()

#endif
