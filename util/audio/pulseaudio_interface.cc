//
// LED Suit Driver - Embedded host driver software for Kevin's LED suit
// controller. Copyright (C) 2019-2020 Kevin Balke
//
// This file is part of LED Suit Driver.
//
// LED Suit Driver is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LED Suit Driver is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LED Suit Driver.  If not, see <http://www.gnu.org/licenses/>.
//

#include "util/audio/pulseaudio_interface.h"

#include <iostream>

namespace opendrop {

namespace {
constexpr static bool kVerboseLogging = false;
}

bool PulseAudioInterface::Initialize() {
  if ((mainloop_ = pa_threaded_mainloop_new()) == nullptr) {
    std::cerr << "Failed to create main loop" << std::endl;
    return false;
  }
  mainloop_api_ = pa_threaded_mainloop_get_api(mainloop_);
  if ((context_ = pa_context_new(mainloop_api_, "LedSuitPaInterface")) ==
      nullptr) {
    std::cerr << "Failed to create context" << std::endl;
    return false;
  }

  pa_context_set_state_callback(context_, ContextStateCallbackStatic, this);

  if (pa_context_connect(
          context_, ((server_name_ == "") ? nullptr : server_name_.c_str()),
          PA_CONTEXT_NOFLAGS, nullptr) < 0) {
    std::cerr << "Faield to connect to server: "
              << pa_strerror(pa_context_errno(context_)) << std::endl;
    return false;
  }

  return true;
}

void PulseAudioInterface::StreamReadCallback(pa_stream *new_stream,
                                             size_t length) {
  MarkSucceeded();
  const void *data;
  while (true) {
    if (pa_stream_peek(new_stream, &data, &length) < 0) {
      std::cerr << "Failed to read from stream" << std::endl;
      MarkFailed();
      return;
    }

    if (length == 0 || data == nullptr) {
      // std::cerr << "Finished" << std::endl;
      return;
    }

    if (kVerboseLogging) {
      std::cout << "Read " << length << " bytes from stream" << std::endl;
    }

    sample_callback_(absl::Span<const float>(
        reinterpret_cast<const float *>(data), length / sizeof(float)));

    if (pa_stream_drop(new_stream) != 0) {
      std::cerr << "Failed to drop frame from stream" << std::endl;
    }
  }
}

void PulseAudioInterface::StreamStateCallback(pa_stream *new_stream) {
  std::cout << "StreamStateCallback" << std::endl;
  auto stream_state = pa_stream_get_state(new_stream);
  switch (stream_state) {
    case PA_STREAM_CREATING:
    case PA_STREAM_TERMINATED:
      break;

    case PA_STREAM_READY:
      const pa_buffer_attr *attributes;
      if ((attributes = pa_stream_get_buffer_attr(new_stream)) == nullptr) {
        std::cerr << "Failed to get stream buffer attributes" << std::endl;
        MarkFailed();
        return;
      }
      std::cout << "Stream buffer attributes: maxlength="
                << attributes->maxlength
                << ", fragsize=" << attributes->fragsize << std::endl;
      break;

    case PA_STREAM_FAILED:
      std::cerr << "Stream error: "
                << pa_strerror(
                       pa_context_errno(pa_stream_get_context(new_stream)))
                << std::endl;
      MarkFailed();
      return;

    default:
      std::cerr << "Unhandled state: " << stream_state << std::endl;
      MarkFailed();
      return;
  }
}

void PulseAudioInterface::ContextStateCallback(pa_context *new_context) {
  std::cout << "ContextStateCallback" << std::endl;
  if (new_context == nullptr) {
    return;
  }

  pa_buffer_attr buffer_attributes{};
  const pa_sample_spec *actual_sample_spec_ptr = nullptr;

  switch (pa_context_get_state(new_context)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
      break;

    case PA_CONTEXT_READY:
      std::cout << "Creating stream with sample spec: format="
                << sample_spec_.format << " rate=" << sample_spec_.rate
                << " channels=" << static_cast<int>(sample_spec_.channels)
                << std::endl;
      if ((stream_ = pa_stream_new(new_context, stream_name_.c_str(),
                                   &sample_spec_, nullptr)) == nullptr) {
        std::cerr << "Failed to create stream" << std::endl;
        MarkFailed();
        return;
      }

      std::cout << "Setting up callbacks" << std::endl;
      pa_stream_set_state_callback(
          stream_, PulseAudioInterface::StreamStateCallbackStatic, this);
      pa_stream_set_read_callback(
          stream_, PulseAudioInterface::StreamReadCallbackStatic, this);

      buffer_attributes.maxlength = -1;
      buffer_attributes.fragsize = 2048;

      std::cout << "Connecting recording stream" << std::endl;
      if (pa_stream_connect_record(stream_, device_name_.c_str(),
                                   &buffer_attributes,
                                   PA_STREAM_ADJUST_LATENCY) < 0) {
        std::cerr << "Failed to connect recording stream: "
                  << pa_strerror(pa_context_errno(new_context)) << std::endl;
        MarkFailed();
        return;
      }

      if ((actual_sample_spec_ptr = pa_stream_get_sample_spec(stream_)) ==
          nullptr) {
        std::cerr << "Unable to read actual sample spec from stream"
                  << std::endl;
        MarkFailed();
        return;
      }
      sample_spec_ = *actual_sample_spec_ptr;
      std::cout << "Sample spec: format=" << sample_spec_.format
                << " rate=" << sample_spec_.rate
                << " channels=" << static_cast<int>(sample_spec_.channels)
                << std::endl;

      MarkSucceeded();

      break;

    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED:
      std::cerr << "PulseAudio connection terminated: "
                << pa_strerror(pa_context_errno(new_context)) << std::endl;
      // TODO: mutex query to get this event
      MarkFailed();
      return;

    case PA_CONTEXT_UNCONNECTED:
      std::cerr << "PulseAudio context unconnected" << std::endl;
      break;
  }
}

bool PulseAudioInterface::Start() {
  return pa_threaded_mainloop_start(mainloop_) == 0;
}

void PulseAudioInterface::Stop() {
  pa_threaded_mainloop_stop(mainloop_);
  pa_threaded_mainloop_free(mainloop_);
}

}  // namespace opendrop
