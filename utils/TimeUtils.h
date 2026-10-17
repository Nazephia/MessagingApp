#ifndef UTILS_TIME_UTILS_H
#define UTILS_TIME_UTILS_H

#include <chrono>
#include <optional>
#include <string>

using TimePoint = std::chrono::system_clock::time_point;

namespace TimeUtils {

// Hora actual truncada a segundos, para que coincida exactamente con lo que se guarda en el CSV.
TimePoint now();

long long toEpochSeconds(const TimePoint& time);
TimePoint fromEpochSeconds(long long seconds);

// Conversión a/desde un campo CSV. Una fecha opcional vacía se guarda como campo vacío.
std::string toCsv(const TimePoint& time);
std::string toCsv(const std::optional<TimePoint>& time);
TimePoint fromCsv(const std::string& field);
std::optional<TimePoint> optionalFromCsv(const std::string& field);

// Formato legible para imprimir: "2026-10-15 14:30:00".
std::string format(const TimePoint& time);

}  // namespace TimeUtils

#endif
