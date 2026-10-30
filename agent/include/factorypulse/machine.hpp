#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "factorypulse/reading.hpp"
#include "factorypulse/sensor.hpp"

namespace fp {

// A machine on the production line. It owns its sensors.
class Machine {
public:
    explicit Machine(std::string id);

    void add_sensor(std::unique_ptr<Sensor> sensor);

    // Reads every sensor once and stamps all readings with the same time.
    std::vector<Reading> read_all(std::chrono::system_clock::time_point now);

    const std::string& id() const noexcept { return id_; }
    std::size_t sensor_count() const noexcept { return sensors_.size(); }

private:
    std::string id_;
    std::vector<std::unique_ptr<Sensor>> sensors_;
};

// A machine with the standard temperature, vibration and pressure sensors.
Machine make_default_machine(const std::string& id, std::uint32_t seed, double fault_probability);

}  // namespace fp
