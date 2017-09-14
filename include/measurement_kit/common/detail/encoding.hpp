// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_ENCODING_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_ENCODING_HPP

#include <measurement_kit/common/error.hpp>
#include <string>

namespace mk {

std::string base64_encode(const std::string &s);

Error utf8_parse(const std::string &s);

} // namespace mk
#endif
