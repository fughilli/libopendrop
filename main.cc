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
#include "backends/imgui_impl_opengl2.h"
#include "backends/imgui_impl_sdl.h"
#include "cleanup.h"
#include "debug/control_injector.h"
#include "debug/signal_scope.h"
#include "gl_interface.h"
#include "gl_texture_manager.h"
#include "imgui.h"
#include "implot.h"
#include "open_drop_controller.h"
#include "open_drop_controller_interface.h"
#include "preset/preset_list.h"
#include "sdl_gl_interface.h"
#include "util/coefficients.h"
#include "util/logging.h"
#include "util/performance_timer.h"
#include "util/pulseaudio_interface.h"
#include "util/rate_limiter.h"

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
ABSL_FLAG(bool, auto_transition, true,
          "Whether or not to transition presets automatically as a function of "
          "the audio input.");
ABSL_FLAG(float, transition_threshold, 2.0f,
          "Some coefficient provided to the transition function. Must be a "
          "positive, nonzero value. Lower values (X => 0) cause faster, more "
          "sensitive transition events. Higher values (X => inf) cause slower, "
          "more conservative transition events.");
ABSL_FLAG(float, transition_cooldown_period, 2.0f,
          "A delay, in seconds, that must elapse in between auto transition "
          "events. Must be positive. A value of 0 disables the cooldown.");
ABSL_FLAG(float, solo_cooldown_period, 120.0f,
          "A delay, in seconds, that must elapse in between transitions to "
          "solo presets.");
ABSL_FLAG(int, max_presets, 2,
          "Maximum number of presets on the screen at a time.");
ABSL_FLAG(int, sampling_rate, 44100,
          "Sampling rate to use with the input source, in Hz.");
ABSL_FLAG(std::string, control_state, "",
          "Path to a .textproto of a ControlState to save to/load from");

namespace opendrop {

namespace {
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
  static RateLimiter<float> solo_rate_limiter{
      absl::GetFlag(FLAGS_solo_cooldown_period)};
  // How many times to attempt to find a preset to add before giving up.
  constexpr int kAttempts = 3;
  int max_presets = absl::GetFlag(FLAGS_max_presets);
  if (max_presets > 0) {
    if (controller->preset_blender()->NumPresets() >= max_presets) {
      return;
    }
  }
  // TODO: Refactor such that preset geometry is configured after attaching to
  // the preset blender.
  float duration = Coefficients::Random<1>(20.0f, 100.0f)[0];
  float ramp_duration = Coefficients::Random<1>(1.0f, 5.0f)[0];
  auto status_or_render_target =
      gl::GlRenderTarget::MakeShared(0, 0, texture_manager);
  if (!status_or_render_target.ok()) {
    LOG(INFO) << "Failed to create render target for preset: "
              << status_or_render_target.status();
    return;
  }
  absl::StatusOr<std::shared_ptr<Preset>> status_or_preset{};
  int attempts = kAttempts;
  while (attempts--) {
    status_or_preset = GetRandomPresetFromList(texture_manager);
    if (!status_or_preset.ok()) {
      LOG(INFO) << "Failed to create preset: " << status_or_preset.status();
      return;
    }
    std::shared_ptr<Preset> preset = status_or_preset.value();
    if (controller->preset_blender()->QueryPresetCount(preset->name()) <
        preset->max_count())
      break;
    LOG(INFO) << "Too many " << preset->name() << "; trying again...";
  }
  if (attempts < 0) {
    LOG(INFO) << "Failed to add preset after " << kAttempts << " attempts.";
    return;
  }

  if (status_or_preset.value()->should_solo() &&
      !solo_rate_limiter.Permitted(controller->global_state().t())) {
    LOG(INFO) << "Blocking preset " << status_or_preset.value()->name()
              << " to keep it from hogging the screen.";
    return;
  }

  controller->preset_blender()->AddPreset(
      *status_or_preset, *status_or_render_target, duration + ramp_duration * 2,
      ramp_duration);
}
}  // namespace

extern "C" int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  absl::InstallFailureSignalHandler(absl::FailureSignalHandlerOptions());

  ControlInjector::SetStatePath(absl::GetFlag(FLAGS_control_state));
  ControlInjector::Load();

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

    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.Fonts->AddFontFromFileTTF("Ubuntu Sans Mono", 12.0f, nullptr,
    // nullptr);
    ImGui_ImplSDL2_InitForOpenGL(sdl_gl_interface->GetWindow().get(),
                                 main_context.get());
    ImGui_ImplOpenGL2_Init();

    LOG(INFO) << "Initializing OpenDrop...";

    auto texture_manager = std::make_shared<gl::GlTextureManager>();
    const int sampling_rate = absl::GetFlag(FLAGS_sampling_rate);

    std::shared_ptr<OpenDropController> open_drop_controller =
        std::make_shared<OpenDropController>(OpenDropController::Options{
            .gl_interface = sdl_gl_interface,
            .texture_manager = texture_manager,
            .sampling_rate = sampling_rate,
            .audio_buffer_size = kAudioBufferSize,
            .width = absl::GetFlag(FLAGS_window_width),
            .height = absl::GetFlag(FLAGS_window_height),
            .draw_output_to_quad = false});
    std::shared_ptr<OpenDropControllerInterface>
        open_drop_controller_interface = open_drop_controller;

    int channel_count = absl::GetFlag(FLAGS_channel_count);

    if (channel_count > 2 || channel_count < 0) {
      LOG(ERROR) << "Unsupported PCM channel count: " << channel_count;
      SDL_Quit();
    }
    auto pa_interface = std::make_shared<PulseAudioInterface>(
        absl::GetFlag(FLAGS_pulseaudio_server),
        absl::GetFlag(FLAGS_pulseaudio_source), "input_stream", sampling_rate,
        channel_count, [&](absl::Span<const float> samples) {
          switch (channel_count) {
            case 1:
              open_drop_controller->audio_processor().AddPcmSamples(
                  PcmFormat::kMono, samples);
              break;
            default:
            case 2:
              open_drop_controller->audio_processor().AddPcmSamples(
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

    bool auto_transition = absl::GetFlag(FLAGS_auto_transition);

    int imgui_width = 300, imgui_height = 300;
    std::vector<float> interleaved_samples;
    std::vector<int> interleaved_samples_indices;
    while (!exit_event_received) {
      // Record the start of the frame in the draw timer.
      auto frame_start_time = absl::GetCurrentTimeNanos() / 1000;
      draw_timer.Start(frame_start_time);

      // Compute the frame time. This is the total elapsed time since the last
      // frame.
      auto frame_time = frame_timer.End(frame_start_time);
      float prev_dt = static_cast<float>(frame_time) / 1000000.0f;
      frame_timer.Start(frame_start_time);

      {
        auto main_context_activation = main_context->Activate();

        bool mouse_moved = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          ImGui_ImplSDL2_ProcessEvent(&event);
          switch (event.type) {
            case SDL_WINDOWEVENT:
              switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
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
            case SDL_MOUSEMOTION:
              mouse_moved = true;
              break;
            case SDL_QUIT:
              exit_event_received = true;
              LOG(INFO) << "EXIT RECEIVED";
              break;
          }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Foo", nullptr, 0);
        ImVec2 wsize = ImGui::GetWindowSize();
        open_drop_controller->UpdateGeometry(wsize.x, wsize.y);

        {
          open_drop_controller->DrawFrame(prev_dt);
          if (auto_transition) {
            static float fire_time = 0.0f;
            if ((open_drop_controller->global_state().t() - fire_time) > 0.5f) {
              // rho is the automatic transition instantaneous power threshold
              // coefficient, or the ratio between the instantaneous power and
              // the average power of the signal required for the `NextPreset`
              // function to be called.
              const static float rho =
                  absl::GetFlag(FLAGS_transition_threshold);
              if (std::abs(
                      open_drop_controller->global_state().power() /
                      open_drop_controller->global_state().average_power()) >
                  rho) {
                fire_time = open_drop_controller->global_state().t();
                static RateLimiter<float> next_preset_limiter(
                    absl::GetFlag(FLAGS_transition_cooldown_period));
                if (next_preset_limiter.Permitted(fire_time)) {
                  NextPreset(open_drop_controller.get(), texture_manager);
                }
              }
            }
          }

          if (open_drop_controller->preset_blender()->NumPresets() == 0) {
            NextPreset(open_drop_controller.get(), texture_manager);
          }
        }

        ImGui::Image((ImTextureID)open_drop_controller->render_target()
                         ->texture_handle(),
                     wsize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        ImGui::Begin("Foo2", nullptr, 0);
        ImPlot::SetNextAxisLimits(ImAxis_Y1, -1.0f, 1.0f);
        if (ImPlot::BeginPlot("samples")) {
          auto &processor = open_drop_controller->audio_processor();
          interleaved_samples.resize(processor.buffer_size() *
                                     processor.channels_per_sample());
          processor.GetSamples(absl::Span<float>(interleaved_samples));
          ImPlot::PlotLine("Interleaved Samples", interleaved_samples.data(),
                           interleaved_samples.size());
          ImPlot::EndPlot();
        }
        SignalScope::Plot();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Handle mouse events

        static Oneshot<float> hide_cursor_oneshot(2.0f);
        static bool cursor_shown = true;

        if (mouse_moved) {
          if (!cursor_shown) {
            cursor_shown = true;
            SDL_ShowCursor(SDL_ENABLE);
          }
          hide_cursor_oneshot.Start(open_drop_controller->global_state().t());
        }
        if (hide_cursor_oneshot.IsDueOnce(
                open_drop_controller->global_state().t())) {
          cursor_shown = false;
          SDL_ShowCursor(SDL_DISABLE);
        }
        // End handle mouse events

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
          SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
          SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
          ImGui::UpdatePlatformWindows();
          ImGui::RenderPlatformWindowsDefault();
          SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        sdl_gl_interface->SwapBuffers();
      }

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

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
  }
  return 0;
}
}  // namespace opendrop
