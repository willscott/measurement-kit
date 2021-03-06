// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/ooni/nettests.hpp>

using namespace mk;

TEST_CASE("Web connectivity works as expected") {
    auto logger = Logger::make();

    SECTION("For website that does not emit standard compliant headers") {
        auto reactor = Reactor::make();
        bool okay = false;
        ooni::web_connectivity("http://mail.voila.fr", Settings{},
                [&](SharedPtr<report::Entry> entry) {
                    okay = ((*entry)["failure"] == nullptr);
                    reactor->stop();
                },
                reactor, logger);
        reactor->run();
        REQUIRE(okay);
    }

    SECTION("For website that advertises upgrade to http2") {
        auto reactor = Reactor::make();
        bool okay = false;
        ooni::web_connectivity("http://www.aseansec.org", Settings{},
                [&](SharedPtr<report::Entry> entry) {
                    okay = ((*entry)["failure"] == nullptr);
                    reactor->stop();
                },
                reactor, logger);
        reactor->run();
        REQUIRE(okay);
    }
}
