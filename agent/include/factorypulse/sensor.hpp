#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

namespace fp {

// Base class for a simulated industrial sensor.
//
// Every read returns: nominal value + a sensor-specific trend + random noise.
// With a small probability the sensor reports a fault value instead, which the
// server should turn into an alarm (overheating, bearing damage, pressure leak...).
class Sensor {
public:
    Sensor(std::string name, std::string unit, double nominal, double noise_stddev,
           double fault_value, double fault_probability, std::uint32_t seed);
    virtual ~Sensor() = default;

    // A sensor owns its random generator state, so copying one would silently
    // duplicate its "future". Sensors are held through std::unique_ptr instead.
    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;

    double read();

    const std::string& name() const noexcept { return name_; }
    const std::string& unit() const noexcept { return unit_; }
    double nominal() const noexcept { return nominal_; }
    double fault_value() const noexcept { return fault_value_; }

protected:
    // Slow, deterministic change over time (e.g. a machine warming up and cooling down).
    virtual double trend(std::size_t tick) const = 0;

private:
    std::string name_;
    std::string unit_;
    double nominal_;
    double fault_value_;
    std::mt19937 rng_;
    std::normal_distribution<double> noise_;
    std::bernoulli_distribution fault_;
    std::size_t tick_ = 0;
};

// Motor housing temperature in °C. Oscillates slowly with the load cycle.
class TemperatureSensor final : public Sensor {
public:
    explicit TemperatureSensor(std::uint32_t seed, double fault_probability = 0.02);

    static constexpr double kNominal = 65.0;
    static constexpr double kNoise = 0.8;
    static constexpr double kFault = 98.0;
    static constexpr double kTrendAmplitude = 3.0;

protected:
    double trend(std::size_t tick) const override;
};

// Vibration velocity (RMS) in mm/s. A fault value means bearing damage.
class VibrationSensor final : public Sensor {
public:
    explicit VibrationSensor(std::uint32_t seed, double fault_probability = 0.02);

    static constexpr double kNominal = 2.5;
    static constexpr double kNoise = 0.3;
    static constexpr double kFault = 11.0;

protected:
    double trend(std::size_t) const override { return 0.0; }
};

// Hydraulic line pressure in bar. A fault value means a leak.
class PressureSensor final : public Sensor {
public:
    explicit PressureSensor(std::uint32_t seed, double fault_probability = 0.02);

    static constexpr double kNominal = 6.0;
    static constexpr double kNoise = 0.1;
    static constexpr double kFault = 2.5;

protected:
    double trend(std::size_t) const override { return 0.0; }
};

}  // namespace fp
