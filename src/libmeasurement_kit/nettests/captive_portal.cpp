// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

CaptivePortalTest::CaptivePortalTest() : BaseTest() {
    runnable.reset(new CaptivePortalRunnable);
    runnable->test_name = "captiveportal";
    runnable->test_version = "0.4.0";
}

void CaptivePortalRunnable::main(std::string input, Settings options,
                                 Callback<SharedPtr<report::Entry>> cb) {
    ooni::captiveportal(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
