#ifndef UTIL_CLEANUP_H_
#define UTIL_CLEANUP_H_

template <typename Fxn>
class Cleanup {
 public:
  Cleanup(Fxn cleanup_fxn) : cleanup_fxn_(cleanup_fxn) {}
  ~Cleanup() { cleanup_fxn_(); }

 private:
  Fxn cleanup_fxn_;
};

template <typename Fxn>
Cleanup<Fxn> MakeCleanup(Fxn cleanup_fxn) {
  return Cleanup(cleanup_fxn);
}

#endif  // UTIL_CLEANUP_H_
