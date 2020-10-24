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

#include "absl/debugging/failure_signal_handler.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/time/clock.h"
#include "absl/types/span.h"
#include "led_driver/performance_timer.h"
#include "led_driver/pulseaudio_interface.h"
#include "libopendrop/cleanup.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/open_drop_controller.h"
#include "libopendrop/open_drop_controller_interface.h"
#include "libopendrop/preset/preset_list.h"
#include "libopendrop/sdl_gl_interface.h"
#include "libopendrop/util/coefficients.h"
#include "libopendrop/util/logging.h"

ABSL_FLAG(std::string, pulseaudio_server, "",
          "PulseAudio server to connect to");
ABSL_FLAG(std::string, pulseaudio_source, "",
          "PulseAudio source device to capture audio from");
ABSL_FLAG(int, channel_count, 2,
          "Audio channel count to request from the audio source");
ABSL_FLAG(int, window_width, 100, "OpenDrop window width");
ABSL_FLAG(int, window_height, 100, "OpenDrop window height");
ABSL_FLAG(int, window_x, -1,
          "OpenDrop window position in x. If this value is -1, no position "
          "override is applied.");
ABSL_FLAG(int, window_y, -1,
          "OpenDrop window position in y. If this value is -1, no position "
          "override is applied.");
ABSL_FLAG(int, late_frames_to_skip_preset, 100,
          "Number of late frames required to skip preset");

namespace led_driver {

namespace {
using ::opendrop::OpenDropController;
using ::opendrop::OpenDropControllerInterface;
using ::opendrop::PcmFormat;
// Target FPS.
constexpr int kFps = 60;
// Target frame time, in microseconds.
constexpr int kTargetFrameTimeUs = 1000000 / kFps;
// Tolerance, in micoseconds, of the frame time measurement before a frame is
// determined to be late.
constexpr int kTargetFrameTimeLateToleranceUs = 100000;
// Size of the audio processor buffer, in samples.
constexpr int kAudioBufferSize = 256;
// Minimum number of milliseconds that should be delayed.
constexpr int kMinimumDelayUs = 2000;

void NextPreset(OpenDropController *controller,
                std::shared_ptr<gl::GlTextureManager> texture_manager) {
  // TODO: Refactor such that preset geometry is configured after attaching to
  // the preset blender.
  std::array<float, 2> durations =
      opendrop::Coefficients::Random<2>(0.2f, 15.0f);
  auto status_or_render_target =
      gl::GlRenderTarget::MakeShared(0, 0, texture_manager);
  if (!status_or_render_target.ok()) {
    LOG(INFO) << "Failed to create render target for preset: "
              << status_or_render_target.status();
    return;
  }
  auto status_or_preset = opendrop::GetRandomPresetFromList(texture_manager);
  if (!status_or_preset.ok()) {
    LOG(INFO) << "Failed to create preset: " << status_or_preset.status();
    return;
  }
  controller->preset_blender()->AddPreset(
      *status_or_preset, *status_or_render_target,
      durations[0] + durations[1] * 2, durations[1]);
}
}  // namespace

extern "C" int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  absl::InstallFailureSignalHandler(absl::FailureSignalHandlerOptions());

  {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      LOG(INFO) << "could not initialize SDL";
      return 1;
    }
    auto sdl_cleanup = MakeCleanup([&] { SDL_Quit(); });

    auto position_x = (absl::GetFlag(FLAGS_window_x) == -1)
                          ? SDL_WINDOWPOS_UNDEFINED
                          : absl::GetFlag(FLAGS_window_x);
    auto position_y = (absl::GetFlag(FLAGS_window_y) == -1)
                          ? SDL_WINDOWPOS_UNDEFINED
                          : absl::GetFlag(FLAGS_window_y);

    auto sdl_gl_interface = std::make_shared<gl::SdlGlInterface>(
        SDL_CreateWindow("OpenDrop", position_x, position_y,
                         absl::GetFlag(FLAGS_window_width),
                         absl::GetFlag(FLAGS_window_height),
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                             SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE));
    sdl_gl_interface->SetVsync(true);

    auto main_context = sdl_gl_interface->AllocateSharedContext();
    auto main_context_activation = main_context->Activate();

    LOG(INFO) << "Initializing OpenDrop...";

    auto texture_manager = std::make_shared<gl::GlTextureManager>();

    std::shared_ptr<OpenDropController> open_drop_controller =
        std::make_shared<OpenDropController>(
            sdl_gl_interface, texture_manager, kAudioBufferSize,
            absl::GetFlag(FLAGS_window_width),
            absl::GetFlag(FLAGS_window_height));
    std::shared_ptr<OpenDropControllerInterface>
        open_drop_controller_interface = open_drop_controller;

    int channel_count = absl::GetFlag(FLAGS_channel_count);

    if (channel_count > 2 || channel_count < 0) {
      LOG(ERROR) << "Unsupported PCM channel count: " << channel_count;
      SDL_Quit();
    }
    auto pa_interface = std::make_shared<PulseAudioInterface>(
        absl::GetFlag(FLAGS_pulseaudio_server),
        absl::GetFlag(FLAGS_pulseaudio_source), "input_stream", channel_count,
        [&](absl::Span<const float> samples) {
          switch (channel_count) {
            case 1:
              open_drop_controller->GetAudioProcessor().AddPcmSamples(
                  PcmFormat::kMono, samples);
              break;
            default:
            case 2:
              open_drop_controller->GetAudioProcessor().AddPcmSamples(
                  PcmFormat::kStereoInterleaved, samples);
              break;
          }
        });
    auto pa_interface_cleanup = MakeCleanup([&] { pa_interface->Stop(); });

    pa_interface->Initialize();
    pa_interface->Start();

    if (!pa_interface->WaitReady()) {
      LOG(ERROR) << "PulseAudio interface failed to initialize.";
      return -1;
    }

    bool exit_event_received = false;
    PerformanceTimer<uint32_t> frame_timer;
    PerformanceTimer<uint32_t> draw_timer;
    int late_frame_counter = 0;
    int late_frames_to_skip_preset =
        absl::GetFlag(FLAGS_late_frames_to_skip_preset);

    while (!exit_event_received) {
      // Record the start of the frame in the draw timer.
      auto frame_start_time = absl::GetCurrentTimeNanos() / 1000;
      draw_timer.Start(frame_start_time);

      // Compute the frame time. This is the total elapsed time since the last
      // frame.
      auto frame_time = frame_timer.End(frame_start_time);
      float prev_dt = static_cast<float>(frame_time) / 1000000.0f;
      frame_timer.Start(frame_start_time);

      open_drop_controller->DrawFrame(prev_dt);

      static float fire_time = 0.0f;
      if ((open_drop_controller->global_state().t() - fire_time) > 0.5f) {
        if (std::abs(open_drop_controller->global_state().power() /
                     open_drop_controller->global_state().average_power()) >
            3.0f) {
          fire_time = open_drop_controller->global_state().t();
          NextPreset(open_drop_controller.get(), texture_manager);
        }
      }

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
                LOG(INFO) << "Next";
                NextPreset(dynamic_cast<OpenDropController *>(
                               open_drop_controller.get()),
                           texture_manager);
                break;
              case SDLK_p:
                LOG(INFO) << "Previous";
                break;
              case SDLK_r:
                LOG(INFO) << "Random";
                break;
              case SDLK_b:
                LOG(INFO) << "Blacklist";
                break;
              case SDLK_w:
                LOG(INFO) << "Whitelist";
                break;
            }
            break;
          case SDL_QUIT:
            exit_event_received = true;
            break;
        }
      }
      sdl_gl_interface->SwapBuffers();

      // Record the end of the draw operations.
      uint32_t draw_time = draw_timer.End(absl::GetCurrentTimeNanos() / 1000);

      static int counter = 0;
      ++counter;
      if (counter == 100) {
        LOG(DEBUG) << "Draw time: " << draw_time
                   << "\tFrame time: " << frame_time
                   << "\tFPS: " << 1 / prev_dt;
        counter = 0;
      }
      if (draw_time >= kTargetFrameTimeUs) {
        if (late_frames_to_skip_preset <= 0) {
          continue;
        }
        if (frame_time >
            (kTargetFrameTimeUs + kTargetFrameTimeLateToleranceUs)) {
          ++late_frame_counter;
          if (late_frame_counter >= late_frames_to_skip_preset) {
            LOG(ERROR) << "Had too many late frames in a row ("
                       << late_frame_counter << "), skipping preset";
            late_frame_counter = 0;
          }
        }
        continue;
      } else {
        late_frame_counter = 0;
      }

      if ((kTargetFrameTimeUs - draw_time) > kMinimumDelayUs) {
        auto delay_time_ms = (kTargetFrameTimeUs - draw_time) / 1000;
        if (delay_time_ms > 0) {
          SDL_Delay(delay_time_ms);
        }
      }
    }
  }
  return 0;
}
}  // namespace led_driver
