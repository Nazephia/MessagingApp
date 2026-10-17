#include "utils/CsvUtils.h"

#include <stdexcept>

namespace CsvUtils {

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

std::vector<std::string> splitLine(const std::string& line, std::size_t fieldCount) {
    std::string clean = line;
    if (!clean.empty() && clean.back() == '\r') {  // archivos guardados en Windows
        clean.pop_back();
    }

    std::vector<std::string> fields;
    std::size_t start = 0;
    while (fields.size() + 1 < fieldCount) {
        std::size_t comma = clean.find(',', start);
        if (comma == std::string::npos) {
            throw std::runtime_error("Línea CSV con menos de " + std::to_string(fieldCount) +
                                     " campos: " + clean);
        }
        fields.push_back(clean.substr(start, comma - start));
        start = comma + 1;
    }
    fields.push_back(clean.substr(start));
    return fields;
}

std::string join(const std::vector<std::string>& parts, char delimiter) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result += delimiter;
        }
        result += parts[i];
    }
    return result;
}

}  // namespace CsvUtils
