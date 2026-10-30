#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <regex>
#include <stdexcept>

#include "factorypulse/machine.hpp"
#include "factorypulse/reading.hpp"

using namespace std::chrono;

TEST(MachineTest, DefaultMachineHasThreeSensors) {
    auto machine = fp::make_default_machine("press-1", 100, 0.0);
    EXPECT_EQ(machine.sensor_count(), 3u);
}

TEST(MachineTest, ReadAllStampsEveryReadingWithMachineIdAndTime) {
    auto machine = fp::make_default_machine("press-1", 100, 0.0);
    const auto now = system_clock::now();

    const auto readings = machine.read_all(now);

    ASSERT_EQ(readings.size(), 3u);
    for (const auto& r : readings) {
        EXPECT_EQ(r.machine_id, "press-1");
        EXPECT_EQ(r.timestamp, now);
    }
    EXPECT_EQ(readings[0].sensor, "temperature");
    EXPECT_EQ(readings[1].sensor, "vibration");
    EXPECT_EQ(readings[2].sensor, "pressure");
}

TEST(MachineTest, RejectsNullSensorAndEmptyId) {
    fp::Machine machine("press-1");
    EXPECT_THROW(machine.add_sensor(nullptr), std::invalid_argument);
    EXPECT_THROW(fp::Machine(""), std::invalid_argument);
}

TEST(ReadingTest, FormatsTimestampAsIso8601Utc) {
    const system_clock::time_point tp{milliseconds(1'500)};
    EXPECT_EQ(fp::to_iso8601(tp), "1970-01-01T00:00:01.500Z");
}

TEST(ReadingTest, SerializesToCamelCaseJson) {
    const fp::Reading r{"press-2", "pressure", "bar", 5.9, system_clock::time_point{}};

    const nlohmann::json j = r;

    EXPECT_EQ(j.at("machineId"), "press-2");
    EXPECT_EQ(j.at("sensor"), "pressure");
    EXPECT_EQ(j.at("unit"), "bar");
    EXPECT_DOUBLE_EQ(j.at("value").get<double>(), 5.9);
    EXPECT_TRUE(std::regex_match(j.at("timestamp").get<std::string>(),
                                 std::regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z)")));
}
