// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/error.hpp>

namespace mk {

Error::Error() : Error(0, "") {}

Error::Error(int e) : Error(e, "") {}

Error::Error(int e, const std::string &r) : code{e}, reason{r} {
    if (code != 0 && reason == "") {
        reason = "unknown_failure " + std::to_string(code);
    }
}

Error::Error(int e, const std::string &r, const Error &c) : Error(e, r) {
    child_errors.push_back(c);
}

Error::operator bool() const { return code != 0; }

bool Error::operator==(int n) const { return code == n; }

bool Error::operator==(Error e) const { return code == e.code; }

bool Error::operator!=(int n) const { return code != n; }

bool Error::operator!=(Error e) const { return code != e.code; }

const char *Error::what() const noexcept { return reason.c_str(); }

void Error::add_child_error(const Error &err) { child_errors.push_back(err); }

} // namespace mk

namespace std {

std::ostream &operator<<(std::ostream &os, const mk::Error &value) {
    os << value.reason;
    return os;
}

} // namespace std
