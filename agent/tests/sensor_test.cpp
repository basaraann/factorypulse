#include <gtest/gtest.h>

#include <vector>

#include "factorypulse/sensor.hpp"

namespace {

template <typename SensorT>
std::vector<double> read_many(SensorT& sensor, int count) {
    std::vector<double> values;
    for (int i = 0; i < count; ++i) {
        values.push_back(sensor.read());
    }
    return values;
}

}  // namespace

TEST(SensorTest, TemperatureStaysInNormalRangeWithoutFaults) {
    fp::TemperatureSensor sensor(42, /*fault_probability=*/0.0);
    // nominal ± trend amplitude ± 6 standard deviations of noise
    const double margin = fp::TemperatureSensor::kTrendAmplitude + 6 * fp::TemperatureSensor::kNoise;

    for (double value : read_many(sensor, 1000)) {
        EXPECT_NEAR(value, fp::TemperatureSensor::kNominal, margin);
    }
}

TEST(SensorTest, AlwaysReportsFaultValueWhenFaultIsCertain) {
    fp::PressureSensor sensor(7, /*fault_probability=*/1.0);

    for (double value : read_many(sensor, 20)) {
        EXPECT_DOUBLE_EQ(value, fp::PressureSensor::kFault);
    }
}

TEST(SensorTest, SameSeedProducesSameSequence) {
    fp::VibrationSensor a(123, 0.1);
    fp::VibrationSensor b(123, 0.1);

    EXPECT_EQ(read_many(a, 200), read_many(b, 200));
}

TEST(SensorTest, DifferentSeedsProduceDifferentSequences) {
    fp::VibrationSensor a(1, 0.0);
    fp::VibrationSensor b(2, 0.0);

    EXPECT_NE(read_many(a, 50), read_many(b, 50));
}

TEST(SensorTest, ExposesNameAndUnit) {
    fp::TemperatureSensor temperature(1);
    fp::VibrationSensor vibration(1);
    fp::PressureSensor pressure(1);

    EXPECT_EQ(temperature.name(), "temperature");
    EXPECT_EQ(temperature.unit(), "C");
    EXPECT_EQ(vibration.name(), "vibration");
    EXPECT_EQ(vibration.unit(), "mm/s");
    EXPECT_EQ(pressure.name(), "pressure");
    EXPECT_EQ(pressure.unit(), "bar");
}
