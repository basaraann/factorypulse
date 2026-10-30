#include "factorypulse/reading.hpp"

#include <cstdio>
#include <ctime>

namespace fp {

std::string to_iso8601(std::chrono::system_clock::time_point tp) {
    using namespace std::chrono;

    const auto since_epoch = tp.time_since_epoch();
    const auto millis = duration_cast<milliseconds>(since_epoch).count() % 1000;
    const std::time_t seconds = system_clock::to_time_t(tp);

    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif

    char date_time[32];
    std::strftime(date_time, sizeof(date_time), "%Y-%m-%dT%H:%M:%S", &utc);

    char result[40];
    std::snprintf(result, sizeof(result), "%s.%03dZ", date_time, static_cast<int>(millis));
    return result;
}

void to_json(nlohmann::json& j, const Reading& r) {
    // camelCase keys so the ASP.NET Core API can bind them without extra configuration.
    j = nlohmann::json{
        {"machineId", r.machine_id},
        {"sensor", r.sensor},
        {"unit", r.unit},
        {"value", r.value},
        {"timestamp", to_iso8601(r.timestamp)},
    };
}

}  // namespace fp
