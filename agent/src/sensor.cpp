#include "factorypulse/sensor.hpp"

#include <cmath>
#include <utility>

namespace fp {

Sensor::Sensor(std::string name, std::string unit, double nominal, double noise_stddev,
               double fault_value, double fault_probability, std::uint32_t seed)
    : name_(std::move(name)),
      unit_(std::move(unit)),
      nominal_(nominal),
      fault_value_(fault_value),
      rng_(seed),
      noise_(0.0, noise_stddev),
      fault_(fault_probability) {}

double Sensor::read() {
    const std::size_t tick = tick_++;
    if (fault_(rng_)) {
        return fault_value_;
    }
    return nominal_ + trend(tick) + noise_(rng_);
}

TemperatureSensor::TemperatureSensor(std::uint32_t seed, double fault_probability)
    : Sensor("temperature", "C", kNominal, kNoise, kFault, fault_probability, seed) {}

double TemperatureSensor::trend(std::size_t tick) const {
    // One full load cycle roughly every 190 readings.
    return kTrendAmplitude * std::sin(static_cast<double>(tick) / 30.0);
}

VibrationSensor::VibrationSensor(std::uint32_t seed, double fault_probability)
    : Sensor("vibration", "mm/s", kNominal, kNoise, kFault, fault_probability, seed) {}

PressureSensor::PressureSensor(std::uint32_t seed, double fault_probability)
    : Sensor("pressure", "bar", kNominal, kNoise, kFault, fault_probability, seed) {}

}  // namespace fp
