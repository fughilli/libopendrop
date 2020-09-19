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

#define _USE_MATH_DEFINES
#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/types/span.h"
#include "led_driver/performance_timer.h"
#include "led_driver/pulseaudio_interface.h"
#include "libopendrop/cleanup.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/open_drop_controller.h"
#include "libopendrop/open_drop_controller_interface.h"
#include "libopendrop/sdl_gl_interface.h"

ABSL_FLAG(std::string, pulseaudio_server, "",
          "PulseAudio server to connect to");
ABSL_FLAG(std::string, pulseaudio_source, "",
          "PulseAudio source device to capture audio from");
ABSL_FLAG(int, channel_count, 2,
          "Audio channel count to request from the audio source");
ABSL_FLAG(int, window_width, 100, "ProjectM window width");
ABSL_FLAG(int, window_height, 100, "ProjectM window height");
ABSL_FLAG(int, late_frames_to_skip_preset, 20,
          "Number of late frames required to skip preset");

namespace led_driver {

namespace {
using ::opendrop::OpenDropController;
using ::opendrop::OpenDropControllerInterface;
using ::opendrop::PcmFormat;
// Target FPS.
constexpr int kFps = 60;
// Target frame time, in milliseconds.
constexpr int kTargetFrameTimeMs = 1000 / kFps;
// Size of the audio processor buffer, in samples.
constexpr int kAudioBufferSize = 256;
struct CallbackData {
  std::shared_ptr<OpenDropControllerInterface> open_drop_controller;
  int channel_count;
};

std::mutex audio_queue_mutex;
std::queue<std::pair<std::shared_ptr<CallbackData>, std::vector<float>>>
    audio_queue;

void AddAudioData(std::shared_ptr<CallbackData> callback_data,
                  absl::Span<const float> samples) {
  switch (callback_data->channel_count) {
    case 1:
      callback_data->open_drop_controller->GetAudioProcessor().AddPcmSamples(
          PcmFormat::kMono, samples);
      break;
    case 2:
      callback_data->open_drop_controller->GetAudioProcessor().AddPcmSamples(
          PcmFormat::kStereoInterleaved, samples);
      break;
    default:
      std::cerr << "Unsupported PCM channel count: "
                << callback_data->channel_count << std::endl;
      SDL_Quit();
  }
}
}  // namespace

extern "C" int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cout << "could not initialize SDL" << std::endl;
      return 1;
    }
    auto sdl_cleanup = MakeCleanup([&] { SDL_Quit(); });

    auto sdl_gl_interface =
        std::make_shared<gl::SdlGlInterface>(SDL_CreateWindow(
            "OpenDrop", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            absl::GetFlag(FLAGS_window_width),
            absl::GetFlag(FLAGS_window_height),
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI |
                SDL_WINDOW_RESIZABLE));
    sdl_gl_interface->SetVsync(true);

    std::cout << "Initializing OpenDrop..." << std::endl;

    std::shared_ptr<OpenDropControllerInterface> open_drop_controller =
        std::make_shared<OpenDropController>(
            sdl_gl_interface, kAudioBufferSize,
            absl::GetFlag(FLAGS_window_width),
            absl::GetFlag(FLAGS_window_height));

    auto callback_data = std::make_shared<CallbackData>();
    callback_data->open_drop_controller = open_drop_controller;
    callback_data->channel_count = absl::GetFlag(FLAGS_channel_count);
    auto pa_interface = std::make_shared<PulseAudioInterface>(
        absl::GetFlag(FLAGS_pulseaudio_server),
        absl::GetFlag(FLAGS_pulseaudio_source), "input_stream",
        callback_data->channel_count,
        [&callback_data](absl::Span<const float> samples) {
          std::lock_guard<std::mutex> audio_queue_lock(audio_queue_mutex);
          audio_queue.push(std::make_pair(
              callback_data,
              std::vector<float>(samples.begin(), samples.end())));
        });
    auto pa_interface_cleanup = MakeCleanup([&] { pa_interface->Stop(); });

    pa_interface->Initialize();
    pa_interface->Start();

    //if (!pa_interface->WaitReady()) {
    //  std::cerr << "PulseAudio interface failed to initialize." << std::endl;
    //  return -1;
    //}

    bool exit_event_received = false;
    PerformanceTimer<uint32_t> frame_timer;
    int late_frame_counter = 0;
    int late_frames_to_skip_preset =
        absl::GetFlag(FLAGS_late_frames_to_skip_preset);
    float prev_dt = 1.0f / kFps;

    auto main_context = sdl_gl_interface->AllocateSharedContext();
    auto main_context_activation = main_context->Activate();
    while (!exit_event_received) {
      frame_timer.Start(SDL_GetTicks());
      {
        std::lock_guard<std::mutex> audio_queue_lock(audio_queue_mutex);
        while (!audio_queue.empty()) {
          AddAudioData(audio_queue.front().first, audio_queue.front().second);
          audio_queue.pop();
        }
      }

      open_drop_controller->DrawFrame(prev_dt);

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_WINDOWEVENT:
            int new_width, new_height;
            SDL_GL_GetDrawableSize(sdl_gl_interface->GetWindow().get(),
                                   &new_width, &new_height);
            switch (event.window.event) {
              case SDL_WINDOWEVENT_RESIZED:
              case SDL_WINDOWEVENT_SIZE_CHANGED:
                open_drop_controller->UpdateGeometry(new_width, new_height);
                break;
            }
            break;
          case SDL_KEYDOWN:
            // Handle key
            switch (event.key.keysym.sym) {
              case SDLK_n:
                std::cout << "Next" << std::endl;
                break;
              case SDLK_p:
                std::cout << "Previous" << std::endl;
                break;
              case SDLK_r:
                std::cout << "Random" << std::endl;
                break;
              case SDLK_b:
                std::cout << "Blacklist" << std::endl;
                break;
              case SDLK_w:
                std::cout << "Whitelist" << std::endl;
                break;
            }
            break;
          case SDL_QUIT:
            exit_event_received = true;
            break;
        }
      }
      sdl_gl_interface->SwapBuffers();

      uint32_t frame_time = frame_timer.End(SDL_GetTicks());
      prev_dt = static_cast<float>(frame_time) / 1000;
      if (frame_time >= kTargetFrameTimeMs) {
        if (late_frames_to_skip_preset <= 0) {
          continue;
        }
        if (frame_time > (kTargetFrameTimeMs + 10)) {
          ++late_frame_counter;
          if (late_frame_counter >= late_frames_to_skip_preset) {
            std::cerr << "Had too many late frames in a row ("
                      << late_frame_counter << "), skipping preset"
                      << std::endl;
            late_frame_counter = 0;
          }
        }
        continue;
      } else {
        late_frame_counter = 0;
      }

      SDL_Delay(kTargetFrameTimeMs - frame_time);
    }
  }
  return 0;
}
}  // namespace led_driver
