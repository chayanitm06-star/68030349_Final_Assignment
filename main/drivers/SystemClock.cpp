#include "SystemClock.hpp"

#include <ctime>

namespace Clock {

bool SystemClock::readUnixTime(int64_t& out) const {
    const time_t now = ::time(nullptr);
    if (now == static_cast<time_t>(-1)) {
        return false;
    }
    out = static_cast<int64_t>(now);
    return true;
}

}  // namespace Clock
