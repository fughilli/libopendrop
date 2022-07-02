#ifndef UTIL_SIGNAL_UNITIZER_H_
#define UTIL_SIGNAL_UNITIZER_H_

namespace opendrop {

class Unitizer {
 public:
  struct Options {
    bool instant_upscale = true;
    float alpha = 0.99f;
  };

  Unitizer(Options options) : options_(options) {}

  float Update(float sample) {
    if (sample > average_ && options_.instant_upscale)
      average_ = sample;
    else
      average_ = average_ * options_.alpha + sample * (1 - options_.alpha);

    if (sample > average_) return 1;

    return sample / average_;
  }

 private:
  float average_ = 0.0f;
  Options options_;
};

}  // namespace opendrop

#endif  // UTIL_SIGNAL_UNITIZER_H_
