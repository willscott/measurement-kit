// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_TRAP_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_TRAP_HPP

#include <functional>
#include <measurement_kit/common/detail/maybe.hpp>
#include <measurement_kit/common/error.hpp>

namespace mk {

Error trap_errors(std::function<void()> &&func);

Maybe<std::exception> trap_exceptions(std::function<void()> &&func);

} // namespace mk
#endif
