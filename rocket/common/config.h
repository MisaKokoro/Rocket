#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <string>

namespace rocket {

class Config {
public:
    Config(const char *xml_path);

    static Config* GetGlobalConfig();
    static void InitGlobalConfig(const char* xmlfile);
public:
    std::string m_log_level;
};


} // namespace rocket
    
#endif