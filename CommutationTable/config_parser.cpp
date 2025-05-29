#include "config_parser.h"


ConfigParser::ConfigParser(const std::string& filename) {
    loadFromFile(filename);
}

void ConfigParser::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        parseLine(line);
    }
}

void ConfigParser::parseLine(const std::string& line) {
    std::string trimmed = line;
    trim(trimmed);

    if (trimmed.empty() || trimmed[0] == '#') {
        return;
    }

    size_t delimiterPos = trimmed.find('=');
    if (delimiterPos == std::string::npos) {
        return;
    }

    std::string key = trimmed.substr(0, delimiterPos);
    std::string value = trimmed.substr(delimiterPos + 1);

    trim(key);
    trim(value);

    if (!key.empty()) {
        config_[key] = value;
    }
}

void ConfigParser::trim(std::string& str) const {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());

    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        str = str.substr(1, str.size() - 2);
    }
}

std::string ConfigParser::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = config_.find(key);
    return (it != config_.end()) ? it->second : defaultValue;
}

int ConfigParser::getInt(const std::string& key, int defaultValue) const {
    auto it = config_.find(key);
    if (it == config_.end()) {
        return defaultValue;
    }

    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}

bool ConfigParser::getBool(const std::string& key, bool defaultValue) const {
    auto it = config_.find(key);
    if (it == config_.end()) {
        return defaultValue;
    }

    std::string value = it->second;
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (value == "true" || value == "TRUE" || value == "1" || value == "yes" || value == "YES") {
        return true;
    }
    if (value == "false" || value == "FALSE" || value == "0" || value == "no" || value == "NO") {
        return false;
    }

    return defaultValue;
}

int ConfigParser::load_ttl_config(const char* filename, ttl_substitution_cfg* cfg) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "enabled = true")) {
            cfg->is_active = true;
        }
        else if (strstr(line, "client_ip = ")) {
            inet_pton(AF_INET, strchr(line, '=') + 2, &cfg->client_ip);
        }
        else if (strstr(line, "server_ip = ")) {
            inet_pton(AF_INET, strchr(line, '=') + 2, &cfg->server_ip);
        }

    }
    fclose(file);
    return 0;
}