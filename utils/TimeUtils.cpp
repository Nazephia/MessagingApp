#include "utils/TimeUtils.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace TimeUtils {

TimePoint now() {
    return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
}

long long toEpochSeconds(const TimePoint& time) {
    return std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
}

TimePoint fromEpochSeconds(long long seconds) {
    return TimePoint(std::chrono::seconds(seconds));
}

std::string toCsv(const TimePoint& time) {
    return std::to_string(toEpochSeconds(time));
}

std::string toCsv(const std::optional<TimePoint>& time) {
    return time.has_value() ? toCsv(*time) : "";
}

TimePoint fromCsv(const std::string& field) {
    return fromEpochSeconds(std::stoll(field));
}

std::optional<TimePoint> optionalFromCsv(const std::string& field) {
    if (field.empty()) {
        return std::nullopt;
    }
    return fromCsv(field);
}

std::string format(const TimePoint& time) {
    std::time_t raw = std::chrono::system_clock::to_time_t(time);
    std::tm local = *std::localtime(&raw);
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

}  // namespace TimeUtils
