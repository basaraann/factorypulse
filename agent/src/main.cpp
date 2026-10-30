// FactoryPulse sensor agent.
//
// Simulates a production line and streams its sensor readings to the
// FactoryPulse API (or to the console with --dry-run).
//
//   fp_agent --api http://localhost:8080 --machines 3 --interval-ms 1000
//   fp_agent --dry-run --machines 1

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "factorypulse/agent.hpp"
#include "factorypulse/machine.hpp"
#include "factorypulse/publisher.hpp"

namespace {

std::atomic<bool> g_running{true};
static_assert(std::atomic<bool>::is_always_lock_free, "needed for use in a signal handler");

void handle_signal(int) { g_running = false; }

struct Options {
    std::string api_url = "http://localhost:8080";
    int machines = 3;
    int interval_ms = 1000;
    double fault_probability = 0.02;
    bool dry_run = false;
};

void print_usage() {
    std::cout << "Usage: fp_agent [--api URL] [--machines N] [--interval-ms MS]\n"
                 "                [--fault-prob P] [--dry-run]\n";
}

Options parse_args(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next_value = [&]() -> std::string {
            if (i + 1 >= argc) {
                throw std::invalid_argument("missing value for " + arg);
            }
            return argv[++i];
        };

        if (arg == "--api") {
            options.api_url = next_value();
        } else if (arg == "--machines") {
            options.machines = std::stoi(next_value());
        } else if (arg == "--interval-ms") {
            options.interval_ms = std::stoi(next_value());
        } else if (arg == "--fault-prob") {
            options.fault_probability = std::stod(next_value());
        } else if (arg == "--dry-run") {
            options.dry_run = true;
        } else if (arg == "--help" || arg == "-h") {
            print_usage();
            std::exit(0);
        } else {
            throw std::invalid_argument("unknown option " + arg);
        }
    }

    if (options.machines < 1 || options.interval_ms < 50 ||
        options.fault_probability < 0.0 || options.fault_probability > 1.0) {
        throw std::invalid_argument("machines >= 1, interval-ms >= 50, 0 <= fault-prob <= 1");
    }
    return options;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    try {
        options = parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        print_usage();
        return 2;
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::vector<fp::Machine> machines;
    for (int i = 1; i <= options.machines; ++i) {
        const std::string id = "press-" + std::to_string(i);
        machines.push_back(fp::make_default_machine(id, static_cast<std::uint32_t>(i * 100),
                                                    options.fault_probability));
    }

    std::unique_ptr<fp::Publisher> publisher;
    if (options.dry_run) {
        publisher = std::make_unique<fp::ConsolePublisher>(std::cout);
    } else {
        publisher = std::make_unique<fp::HttpPublisher>(options.api_url);
    }

    fp::Agent agent(std::move(machines), *publisher);
    std::cerr << "FactoryPulse agent started: " << options.machines << " machine(s), every "
              << options.interval_ms << " ms -> "
              << (options.dry_run ? std::string("console") : options.api_url) << '\n';

    while (g_running) {
        const auto sent = agent.tick(std::chrono::system_clock::now());
        if (sent == 0 && !options.dry_run) {
            std::cerr << "server unreachable, " << agent.pending() << " reading(s) buffered\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(options.interval_ms));
    }

    std::cerr << "agent stopped (" << agent.pending() << " unsent, " << agent.dropped()
              << " dropped)\n";
    return 0;
}
