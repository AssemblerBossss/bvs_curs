#pragma once
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "../Headers.h"

struct ttl_substitution_cfg {
    bool is_active;
    struct in_addr client_ip;
    struct in_addr server_ip;
    uint8_t max_ttl;
};

class ConfigParser {
public:
    explicit ConfigParser(const std::string& filename);

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    bool isLoaded() const { return !config_.empty(); }

private:
    void loadFromFile(const std::string& filename);
    void parseLine(const std::string& line);
    void trim(std::string& str) const;


    int load_ttl_config(const char* filename, ttl_substitution_cfg* cfg);
    std::map<std::string, std::string> config_;
};

#endif // CONFIG_PARSER_H