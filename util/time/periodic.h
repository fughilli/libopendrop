#ifndef UTIL_TIME_PERIODIC_H_
#define UTIL_TIME_PERIODIC_H_

template <typename T>
class Periodic {
 public:
  // Constructs a oneshot timer with the given duration.
  explicit Periodic(T duration) : duration_(duration), deadline_(0) {}

  // Starts the oneshot timer at time `start_time`.
  void Start(T start_time) { deadline_ = start_time + duration_; }

  // Checks if the timer is due, given the current time.
  bool IsDue(T current_time) { return MaybeUpdateDeadline(current_time); }

  // Gets the phase of the current time against this timer's period. If the
  // timer just fired, this value is 0. At the time that the timer fires, this
  // value is 1.
  float FractionDue(T current_time) {
    MaybeUpdateDeadline(current_time);
    return static_cast<float>(deadline_ - current_time) / duration_;
  }

 private:
  bool MaybeUpdateDeadline(T current_time) {
    if (current_time < deadline_) return false;

    if (current_time < deadline_ + duration_) {
      deadline_ += duration_;
      return true;
    }

    // Slow path
    T cycles_ahead = (current_time - deadline_) / duration_;
    deadline_ += (cycles_ahead + 1) * duration_;
    return true;
  }
  T duration_;
  T deadline_;
};

#endif  // UTIL_TIME_PERIODIC_H_
