#include "factorypulse/publisher.hpp"

#include <httplib.h>

namespace fp {

bool ConsolePublisher::publish(const std::vector<Reading>& readings) {
    out_ << nlohmann::json(readings).dump() << '\n';
    return static_cast<bool>(out_);
}

HttpPublisher::HttpPublisher(const std::string& base_url)
    : client_(std::make_unique<httplib::Client>(base_url)) {
    client_->set_connection_timeout(2);  // seconds
    client_->set_read_timeout(5);
}

HttpPublisher::~HttpPublisher() = default;

bool HttpPublisher::publish(const std::vector<Reading>& readings) {
    const auto body = nlohmann::json(readings).dump();
    const auto response = client_->Post("/api/readings", body, "application/json");
    return response && response->status >= 200 && response->status < 300;
}

}  // namespace fp
