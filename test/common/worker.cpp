// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "private/ext/catch.hpp"
#include <chrono>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/common/detail/worker.hpp>
#include <thread>

TEST_CASE("The worker is robust to submitting many tasks in a row") {
    auto worker = mk::Var<mk::Worker>::make();
    for (int i = 0; i < 128; ++i) {
        worker->run_in_background_thread([]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        });
    }
    for (;;) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        auto concurrency = worker->concurrency();
        std::cout << "Concurrency: " << concurrency << "\n";
        REQUIRE(concurrency <= worker->parallelism());
        if (concurrency == 0) {
            break;
        }
    }
}
