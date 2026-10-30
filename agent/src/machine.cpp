#include "factorypulse/machine.hpp"

#include <stdexcept>
#include <utility>

namespace fp {

Machine::Machine(std::string id) : id_(std::move(id)) {
    if (id_.empty()) {
        throw std::invalid_argument("machine id must not be empty");
    }
}

void Machine::add_sensor(std::unique_ptr<Sensor> sensor) {
    if (!sensor) {
        throw std::invalid_argument("sensor must not be null");
    }
    sensors_.push_back(std::move(sensor));
}

std::vector<Reading> Machine::read_all(std::chrono::system_clock::time_point now) {
    std::vector<Reading> readings;
    readings.reserve(sensors_.size());
    for (const auto& sensor : sensors_) {
        readings.push_back(Reading{id_, sensor->name(), sensor->unit(), sensor->read(), now});
    }
    return readings;
}

Machine make_default_machine(const std::string& id, std::uint32_t seed, double fault_probability) {
    Machine machine(id);
    // Different seeds per sensor so the three signals are independent.
    machine.add_sensor(std::make_unique<TemperatureSensor>(seed, fault_probability));
    machine.add_sensor(std::make_unique<VibrationSensor>(seed + 1, fault_probability));
    machine.add_sensor(std::make_unique<PressureSensor>(seed + 2, fault_probability));
    return machine;
}

}  // namespace fp
