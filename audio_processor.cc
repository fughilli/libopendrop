#include "audio_processor.h"

namespace opendrop {

AudioProcessor::AudioProcessor(ptrdiff_t buffer_size)
    : buffer_size_(buffer_size) {
  samples_interleaved_.resize(buffer_size_ * kChannelsPerSample, 0);
}

void AudioProcessor::AddPcmSamples(PcmFormat format,
                                   absl::Span<const float> samples) {
  if (format == PcmFormat::kMono) {
    std::vector<float> intermediate_buffer;
    intermediate_buffer.resize(samples.size() * 2);
    for (int i = 0; i < samples.size(); ++i) {
      intermediate_buffer[i * 2] = samples[i];
      intermediate_buffer[i * 2 + 1] = samples[i];
    }
    AddPcmSamples(PcmFormat::kStereoInterleaved,
                  absl::Span<const float>(intermediate_buffer));
    return;
  }

  std::unique_lock<std::mutex> samples_lock(samples_interleaved_mu_);

  // if (samples.size() >= samples_interleaved_.size()) {
  //   std::copy(samples.begin() + (samples.size() -
  //   samples_interleaved_.size()),
  //             samples.end(), samples_interleaved_.begin());
  //   return;
  // }

  // // Move samples forward in buffer
  // std::copy(samples_interleaved_.begin() + samples.size(),
  //           samples_interleaved_.end(), samples_interleaved_.begin());
  // // Append new samples to end of old samples
  // std::copy(samples.begin(), samples.end(),
  //           samples_interleaved_.begin() +
  //               (samples_interleaved_.size() - samples.size()));
  samples_interleaved_.reserve(samples_interleaved_.size() + samples.size());
  samples_interleaved_.insert(samples_interleaved_.end(), samples.begin(),
                              samples.end());
}

bool AudioProcessor::GetSamples(absl::Span<float>& out_samples) {
  std::unique_lock<std::mutex> lock(samples_interleaved_mu_);
  if (out_samples.size() < samples_interleaved_.size()) {
    return false;
  }
  std::copy(samples_interleaved_.begin(), samples_interleaved_.end(),
            out_samples.begin());
  out_samples =
      absl::Span<float>{out_samples.data(), samples_interleaved_.size()};
  samples_interleaved_.resize(0);
  return true;
}

}  // namespace opendrop
