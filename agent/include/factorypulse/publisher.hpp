#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "factorypulse/reading.hpp"

namespace httplib {
class Client;
}

namespace fp {

// Where readings go. The agent only depends on this interface, so tests can
// plug in a fake and the real program can choose HTTP or console output.
class Publisher {
public:
    virtual ~Publisher() = default;

    // Returns true if every reading was accepted.
    virtual bool publish(const std::vector<Reading>& readings) = 0;
};

// Prints each batch as one JSON line. Useful for running the agent without a server.
class ConsolePublisher final : public Publisher {
public:
    explicit ConsolePublisher(std::ostream& out) : out_(out) {}
    bool publish(const std::vector<Reading>& readings) override;

private:
    std::ostream& out_;
};

// POSTs each batch as a JSON array to <base_url>/api/readings.
class HttpPublisher final : public Publisher {
public:
    explicit HttpPublisher(const std::string& base_url);
    ~HttpPublisher() override;  // defined in the .cpp, where httplib::Client is complete

    bool publish(const std::vector<Reading>& readings) override;

private:
    std::unique_ptr<httplib::Client> client_;
};

}  // namespace fp
