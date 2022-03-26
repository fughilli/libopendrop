#ifndef UTIL_TIME_ONESHOT_H_
#define UTIL_TIME_ONESHOT_H_

namespace opendrop {

template <typename T>
class Oneshot {
 public:
  // Constructs a oneshot timer with the given duration.
  explicit Oneshot(T duration)
      : duration_(duration), deadline_(0), fired_(false) {}

  // Starts the oneshot timer at time `start_time`.
  void Start(T start_time) {
    deadline_ = start_time + duration_;
    fired_ = false;
  }

  // Checks if the timer is due, given the current time.
  bool IsDue(T current_time) const { return current_time >= deadline_; }

  // Checks if the timer is due, given the current time. Will only return true
  // at the first time IsDueOnce() is called after the timer becomes due.
  bool IsDueOnce(T current_time) {
    if (fired_) return false;
    return (fired_ = (current_time >= deadline_));
  }

  // Gets the fraction of elapsed time on this timer. If the timer was just
  // started, this value is 0. If the timer is due, this value is 1.
  float FractionDue(T current_time) const {
    return static_cast<float>(deadline_ - current_time) / duration_;
  }

 private:
  T duration_;
  T deadline_;
  bool fired_;
};

template <typename T>
class OneshotIncremental {
 public:
  // Constructs an incremental-update oneshot timer with the given duration.
  explicit OneshotIncremental(T duration)
      : duration_(duration), current_time_(0) {}

  // Resets the oneshot timer.
  void Reset() {
    current_time_ = 0;
    fired_ = false;
  }

  // Updates the elapsed time on this timer, given the delta time since the last
  // update.
  OneshotIncremental& Update(T delta_time) {
    current_time_ += delta_time;
    return *this;
  }

  // Checks if the timer is due.
  bool IsDue() { return current_time_ >= duration_; }

  // Checks if the timer is due. Will only return true at the first time
  // IsDueOnce() is called after the timer becomes due.
  bool IsDueOnce() {
    if (fired_) return false;
    return (fired_ = (current_time_ >= duration_));
  }

  // Gets the fraction of elapsed time on this timer. If the timer was just
  // started, this value is 0. If the timer is due, this value is 1.
  float FractionDue() const {
    return static_cast<float>(current_time_) / duration_;
  }

 private:
  T duration_;
  T current_time_;
  bool fired_;
};

}  // namespace opendrop

#endif  // UTIL_TIME_ONESHOT_H_
