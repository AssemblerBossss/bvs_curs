#include "../Headers.h"

#ifndef CURSOV_CONFIG_H
#define CURSOV_CONFIG_H

/**
 * @brief Проверяет существование и доступность файла для чтения.
 * @param filename Путь к проверяемому файлу.
 * @return true, если файл существует и доступен для чтения.
 */
inline bool file_exists_and_readable(const std::string& filename) {
    std::filesystem::path file_path(filename);

    return std::filesystem::exists(file_path) &&           // Проверка существования файла
           std::filesystem::is_regular_file(file_path) &&  // Исключает каталоги, устройства и т.д.;
           (access(filename.c_str(), R_OK) == 0);  // Проверка, доступен ли файл для чтения текущим пользователем
}

/**
 * @brief Читает конфигурационный файл формата key=value.
 * @param filename Путь к конфигурационному файлу.
 * @return Словарь с ключами и значениями из конфигурационного файла.
 * @throws std::runtime_error Если файл не существует или недоступен для чтения.
 */
inline std::unordered_map<std::string, std::string> read_config_file(const std::string& filename) {
    if (!file_exists_and_readable(filename)) {
        throw std::runtime_error("Файл конфигурации недоступен или не существует: " + filename);
    }

    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    while(std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;

        // Пропускаем строки без разделителя или пустые строки
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Удаление пробелов вокруг ключа и значения
            key.erase(0, key.find_first_not_of(" \t\n\r"));
            key.erase(key.find_last_not_of(" \t\n\r") + 1);
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            value.erase(value.find_last_not_of(" \t\n\r") + 1);

            if (!key.empty() && !value.empty()) {
                config[key] = value;
            }
        }
    }

    return config;
}



#endif //CURSOV_CONFIG_H
