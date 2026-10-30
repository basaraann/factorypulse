#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <vector>

#include "factorypulse/machine.hpp"
#include "factorypulse/publisher.hpp"
#include "factorypulse/reading.hpp"

namespace fp {

// Collects readings from all machines and sends them through a Publisher.
//
// If the server is unreachable, readings are kept in a bounded buffer and sent
// on the next successful tick, so a short network outage loses no data. When
// the buffer is full the oldest readings are dropped first.
class Agent {
public:
    Agent(std::vector<Machine> machines, Publisher& publisher, std::size_t max_pending = 10'000);

    // Reads every sensor once, then tries to publish everything pending.
    // Returns how many readings were delivered (0 if publishing failed).
    std::size_t tick(std::chrono::system_clock::time_point now);

    std::size_t pending() const noexcept { return pending_.size(); }
    std::size_t dropped() const noexcept { return dropped_; }

private:
    std::vector<Machine> machines_;
    Publisher& publisher_;
    std::size_t max_pending_;
    std::deque<Reading> pending_;
    std::size_t dropped_ = 0;
};

}  // namespace fp
