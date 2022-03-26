#ifndef UTIL_AUDIO_AUDIO_PROCESSOR_H_
#define UTIL_AUDIO_AUDIO_PROCESSOR_H_

#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

#include "absl/types/span.h"

namespace opendrop {

enum class PcmFormat : int {
  kMono = 0,
  kStereoInterleaved = 1,
};

// AudioProcessor takes either mono or stereo audio floating point samples and
// adds them to a sliding buffer of a fixed size. Samples in this buffer are
// interleaved stereo, regardless of the input format.
//
// AudioProcessor also performs additional processing on the audio, providing
// signal energy, spectrum, and filtered audio with different FIR and IIR
// filters.
//
// This class is thread-safe.
class AudioProcessor {
 public:
  // Constructs an AudioProcessor with the given buffer size.
  AudioProcessor(ptrdiff_t buffer_size);

  // Adds PCM samples to the audio buffer.
  void AddPcmSamples(PcmFormat format, absl::Span<const float> samples);

  // Copies the current sample buffer into `out_samples`.
  bool GetSamples(absl::Span<float>& out_samples);

  ptrdiff_t buffer_size() const { return buffer_size_; }
  int channels_per_sample() const { return kChannelsPerSample; }

 private:
  // Number of channels per sample in the sample buffer.
  static constexpr int kChannelsPerSample = 2;

  // Size of the sample buffer, in samples.
  const ptrdiff_t buffer_size_;

  // Mutex guarding the sample buffer.
  std::mutex samples_interleaved_mu_;

  // Sample buffer. Samples are stored interleaved: [L,R,L,R,...]
  std::vector<float> samples_interleaved_;
};

}  // namespace opendrop

#endif  // UTIL_AUDIO_AUDIO_PROCESSOR_H_
