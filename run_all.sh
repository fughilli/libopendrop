#!/bin/bash

source=microphone
run_left=1
run_right=1
run_manual_controlled=1

function find_window() {
  name="$1"
  class="$2"

  comm -12 \
    <(xdotool search --name "$name" | sort -u) \
    <(xdotool search --class "$class" | sort -u)
}

if [[ $run_left == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=left_eye ./run_libopendrop.sh \
    -B binaries/libopendrop_latest_ct_eyes_signals_optional \
    -s $source \
    --nodraw_signal_viewer \
    --control_state=$(pwd)/configs/interactive_eye_left.textproto \
    --control_port=9955 \
    --inject &
fi

if [[ $run_right == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=right_eye ./run_libopendrop.sh \
    -B binaries/libopendrop_latest_ct_eyes_signals_optional \
    -s $source \
    --nodraw_signal_viewer \
    --control_state=$(pwd)/configs/interactive_eye_right.textproto \
    --control_port=9945 \
    --inject &
fi

if [[ $run_manual_controlled == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=manual_control ./run_libopendrop.sh \
    -B binaries/libopendrop_for_ct_live_default_control_enabled \
    -s $source \
    --control_state=$(pwd)/configs/mpk_mini_config.textproto &
  bin_pid=$!
fi

bazelisk run //debug:control_sender -- --input_filter='.*Feather.*' --ports=9955 --ports=9945 &
bazelisk run //debug:control_sender -- --input_filter='.*MPK.*' --ports=9944 --ports=9955 --ports=9945 &

if [[ $run_left == 1 ]]; then
  while [[ -z $(xdotool search --class "left_eye") ]]; do
    echo "Waiting for left_eye"
    sleep 1
  done
fi

if [[ $run_right == 1 ]]; then
  while [[ -z $(xdotool search --class "right_eye") ]]; do
    echo "Waiting for right_eye"
    sleep 1
  done
fi

if [[ $run_manual_controlled == 1 ]]; then
  while [[ -z $(xdotool search --class "manual_control") ]]; do
    echo "Waiting for manual_control"
    sleep 1
  done
fi

sleep 2

LEFT_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "left_eye")
RIGHT_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "right_eye")
MANUAL_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "manual_control")

echo $LEFT_WINDOW_ID $RIGHT_WINDOW_ID $MANUAL_WINDOW_ID

i3-msg "[id=\"$MANUAL_WINDOW_ID\"] move container to output HDMI-0"
i3-msg "[id=\"$MANUAL_WINDOW_ID\"] fullscreen enable"

sleep 2
i3-msg "[id=\"$LEFT_WINDOW_ID\"] move container to output DP-0.1"
i3-msg "[id=\"$LEFT_WINDOW_ID\"] fullscreen enable"

sleep 2
i3-msg "[id=\"$RIGHT_WINDOW_ID\"] move container to output DP-0.2"
i3-msg "[id=\"$RIGHT_WINDOW_ID\"] fullscreen enable"

wait

# if [[ $run_manual_controlled == 1 ]]; then
#   kill $bin_pid
#   killall libopendrop_for_ct_live
# fi

killall binaries/*
