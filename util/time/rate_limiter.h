#ifndef UTIL_RATE_LIMITER_H_
#define UTIL_RATE_LIMITER_H_

#include "util/time/oneshot.h"

namespace opendrop {

template <typename T>
class RateLimiter {
 public:
  explicit RateLimiter(T period) : oneshot_(period) {}

  void Start(T start_time) { oneshot_.Start(start_time); }
  bool Permitted(T current_time) {
    if (!oneshot_.IsDue(current_time)) {
      return false;
    }
    oneshot_.Start(current_time);
    return true;
  }

 private:
  Oneshot<T> oneshot_;
};

template <typename T>
class RateLimiterIncremental {
 public:
  explicit RateLimiterIncremental(T period) : oneshot_(period) {}

  bool Permitted(T delta_time) {
    oneshot_.Update(delta_time);
    if (!oneshot_.IsDue()) {
      return false;
    }
    oneshot_.Reset();
    return true;
  }

 private:
  OneshotIncremental<T> oneshot_;
};

}  // namespace opendrop

#endif  // UTIL_RATE_LIMITER_H_
