// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/libevent/reactor.hpp>

namespace mk {

/*static*/ SharedPtr<Reactor> Reactor::make() {
    return std::dynamic_pointer_cast<Reactor>(
        std::make_shared<libevent::Reactor<>>());
}

Reactor::~Reactor() {}

void Reactor::run_with_initial_event(Callback<> &&cb) {
    call_soon(std::move(cb));
    run();
}

/*static*/ SharedPtr<Reactor> Reactor::global() {
    static SharedPtr<Reactor> singleton = make();
    return singleton;
}

} // namespace mk
