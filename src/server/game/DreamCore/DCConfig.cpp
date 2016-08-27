#include <algorithm>
#include <boost/property_tree/ini_parser.hpp>
#include "DCConfig.h"
#include "Log.h"

using namespace std;
using namespace boost::property_tree;

bool DCConfigMgr::Load(const string& file)
{
    lock_guard<mutex> lock(m_configLock);

    m_filename = file;

    try
    {
        ptree tree;
        ini_parser::read_ini(file, tree);

        if (tree.empty())
        {
            auto error = "empty file (" + file + ")";
            TC_LOG_ERROR("dc.config", "Could not load config file: %s", error.c_str());
            return false;
        }

        m_config = tree.begin()->second;
    }
    catch (ini_parser::ini_parser_error const& e)
    {
        auto error = e.line() == 0 ? e.message() + " (" + e.filename() + ")" : e.message() + " (" + e.filename() + ":" + to_string(e.line()) + ")";
        TC_LOG_ERROR("dc.config", "Config load failed: %s", error.c_str());
        return false;
    }

    return true;
}

DCConfigMgr* DCConfigMgr::instance()
{
    static DCConfigMgr instance;
    return &instance;
}

bool DCConfigMgr::Reload()
{
    return Load(m_filename);
}

template<class T>
T DCConfigMgr::GetValueDefault(string const& name, T def) const
{
    try
    {
        return m_config.get<T>(ptree::path_type(name, '/'));
    }
    catch (ptree_bad_path)
    {
        TC_LOG_WARN("dc.config", "Missing name %s in config file %s, add \"%s = %s\" to this file", name.c_str(), m_filename.c_str(), name.c_str(), to_string(def).c_str());
    }
    catch (ptree_bad_data)
    {
        TC_LOG_ERROR("dc.config", "Bad value defined for name %s in config file %s, going to use %s instead", name.c_str(), m_filename.c_str(), to_string(def).c_str());
    }

    return def;
}

template<>
string DCConfigMgr::GetValueDefault<string>(string const& name, string def) const
{
    try
    {
        return m_config.get<string>(ptree::path_type(name, '/'));
    }
    catch (ptree_bad_path)
    {
        TC_LOG_WARN("dc.config", "Missing name %s in config file %s, add \"%s = %s\" to this file", name.c_str(), m_filename.c_str(), name.c_str(), def.c_str());
    }
    catch (ptree_bad_data)
    {
        TC_LOG_ERROR("dc.config", "Bad value defined for name %s in config file %s, going to use %s instead", name.c_str(), m_filename.c_str(), def.c_str());
    }

    return def;
}

string DCConfigMgr::GetStringDefault(string const& name, const string& def) const
{
    auto val = GetValueDefault(name, def);
    val.erase(remove(val.begin(), val.end(), '"'), val.end());
    return val;
}

bool DCConfigMgr::GetBoolDefault(string const& name, bool def) const
{
    auto val = GetValueDefault(name, string(def ? "1" : "0"));
    val.erase(remove(val.begin(), val.end(), '"'), val.end());
    return (val == "1" || val == "true" || val == "TRUE" || val == "yes" || val == "YES");
}

int DCConfigMgr::GetIntDefault(string const& name, int def) const
{
    return GetValueDefault(name, def);
}

float DCConfigMgr::GetFloatDefault(string const& name, float def) const
{
    return GetValueDefault(name, def);
}

string const& DCConfigMgr::GetFilename()
{
    lock_guard<mutex> lock(m_configLock);
    return m_filename;
}

list<string> DCConfigMgr::GetKeysByString(string const& name)
{
    lock_guard<mutex> lock(m_configLock);

    list<string> keys;

    for (const auto& child : m_config)
        if (child.first.compare(0, name.length(), name) == 0)
            keys.push_back(child.first);

    return keys;
}
