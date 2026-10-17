#ifndef UTILS_CSV_UTILS_H
#define UTILS_CSV_UTILS_H

#include <cstddef>
#include <string>
#include <vector>

namespace CsvUtils {

// Divide `text` por `delimiter`, conservando los campos vacíos ("a||b" -> "a", "", "b").
std::vector<std::string> split(const std::string& text, char delimiter);

// Divide una línea CSV en exactamente `fieldCount` campos separados por coma.
// El último campo se queda con el resto de la línea, así que puede contener comas.
// Lanza std::runtime_error si la línea tiene menos campos.
std::vector<std::string> splitLine(const std::string& line, std::size_t fieldCount);

std::string join(const std::vector<std::string>& parts, char delimiter);

}  // namespace CsvUtils

#endif
