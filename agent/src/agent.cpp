#include "factorypulse/agent.hpp"

#include <stdexcept>
#include <utility>

namespace fp {

Agent::Agent(std::vector<Machine> machines, Publisher& publisher, std::size_t max_pending)
    : machines_(std::move(machines)), publisher_(publisher), max_pending_(max_pending) {
    if (max_pending_ == 0) {
        throw std::invalid_argument("max_pending must be greater than zero");
    }
}

std::size_t Agent::tick(std::chrono::system_clock::time_point now) {
    for (auto& machine : machines_) {
        for (auto& reading : machine.read_all(now)) {
            pending_.push_back(std::move(reading));
        }
    }

    while (pending_.size() > max_pending_) {
        pending_.pop_front();
        ++dropped_;
    }

    const std::vector<Reading> batch(pending_.begin(), pending_.end());
    if (!publisher_.publish(batch)) {
        return 0;
    }
    pending_.clear();
    return batch.size();
}

}  // namespace fp
