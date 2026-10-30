#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include "factorypulse/agent.hpp"

namespace {

// Records what it was asked to publish and can be told to fail.
class FakePublisher final : public fp::Publisher {
public:
    bool fail = false;
    std::vector<std::vector<fp::Reading>> batches;

    bool publish(const std::vector<fp::Reading>& readings) override {
        if (fail) {
            return false;
        }
        batches.push_back(readings);
        return true;
    }
};

std::vector<fp::Machine> two_machines() {
    std::vector<fp::Machine> machines;
    machines.push_back(fp::make_default_machine("press-1", 100, 0.0));
    machines.push_back(fp::make_default_machine("press-2", 200, 0.0));
    return machines;
}

const auto kNow = std::chrono::system_clock::now();

}  // namespace

TEST(AgentTest, PublishesAllReadingsOfAllMachines) {
    FakePublisher publisher;
    fp::Agent agent(two_machines(), publisher);

    EXPECT_EQ(agent.tick(kNow), 6u);  // 2 machines x 3 sensors
    ASSERT_EQ(publisher.batches.size(), 1u);
    EXPECT_EQ(publisher.batches[0].size(), 6u);
    EXPECT_EQ(agent.pending(), 0u);
}

TEST(AgentTest, BuffersReadingsWhileServerIsDownAndSendsThemLater) {
    FakePublisher publisher;
    fp::Agent agent(two_machines(), publisher);

    publisher.fail = true;
    EXPECT_EQ(agent.tick(kNow), 0u);
    EXPECT_EQ(agent.tick(kNow), 0u);
    EXPECT_EQ(agent.pending(), 12u);

    publisher.fail = false;
    EXPECT_EQ(agent.tick(kNow), 18u);  // 12 buffered + 6 new
    EXPECT_EQ(agent.pending(), 0u);
}

TEST(AgentTest, DropsOldestReadingsWhenBufferIsFull) {
    FakePublisher publisher;
    fp::Agent agent(two_machines(), publisher, /*max_pending=*/10);

    publisher.fail = true;
    agent.tick(kNow);  // 6 pending
    agent.tick(kNow);  // 12 -> trimmed to 10

    EXPECT_EQ(agent.pending(), 10u);
    EXPECT_EQ(agent.dropped(), 2u);
}

TEST(AgentTest, RejectsZeroBufferSize) {
    FakePublisher publisher;
    EXPECT_THROW(fp::Agent(two_machines(), publisher, 0), std::invalid_argument);
}
