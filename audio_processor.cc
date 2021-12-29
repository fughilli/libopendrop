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

  if (samples.size() >= samples_interleaved_.size()) {
    std::copy(samples.begin() + (samples.size() - samples_interleaved_.size()),
              samples.end(), samples_interleaved_.begin());
    return;
  }

  // Move samples forward in buffer
  std::copy(samples_interleaved_.begin() + samples.size(),
            samples_interleaved_.end(), samples_interleaved_.begin());
  // Append new samples to end of old samples
  std::copy(samples.begin(), samples.end(),
            samples_interleaved_.begin() +
                (samples_interleaved_.size() - samples.size()));
}

bool AudioProcessor::GetSamples(absl::Span<float> out_samples) {
  if (out_samples.size() != buffer_size_ * kChannelsPerSample) {
    return false;
  }
  std::unique_lock<std::mutex> lock(samples_interleaved_mu_);
  std::copy(samples_interleaved_.begin(), samples_interleaved_.end(),
            out_samples.begin());
  return true;
}

}  // namespace opendrop
