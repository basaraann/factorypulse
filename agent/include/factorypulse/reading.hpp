#pragma once

#include <chrono>
#include <string>

#include <nlohmann/json.hpp>

namespace fp {

// One measurement taken from one sensor of one machine.
struct Reading {
    std::string machine_id;
    std::string sensor;
    std::string unit;
    double value = 0.0;
    std::chrono::system_clock::time_point timestamp;
};

// Formats a time point as ISO-8601 UTC with milliseconds, e.g. "2026-10-30T03:15:42.123Z".
std::string to_iso8601(std::chrono::system_clock::time_point tp);

// nlohmann::json picks this up automatically (ADL), so `nlohmann::json j = readings;`
// works for a single Reading as well as for a std::vector<Reading>.
void to_json(nlohmann::json& j, const Reading& r);

}  // namespace fp
