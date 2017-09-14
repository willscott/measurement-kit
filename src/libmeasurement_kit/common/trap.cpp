// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/detail/trap.hpp>

namespace mk {

Error trap_errors(std::function<void()> &&fun) {
    Error error;
    try {
        fun();
    } catch (const Error &exc) {
        error = exc;
    }
    return error;
}

Maybe<std::exception> trap_exceptions(std::function<void()> &&fun) {
    try {
        fun();
    } catch (std::exception &exc) {
        return std::move(exc);
    }
    return {};
}

} // namespace mk
