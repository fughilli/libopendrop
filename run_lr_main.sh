#!/bin/bash

source=microphone
run_left=0
run_right=0
run_manual_controlled=1
kill_needed=0

pids_to_kill=()

function find_window() {
  name="$1"
  class="$2"

  comm -12 \
    <(xdotool search --name "$name" | sort -u) \
    <(xdotool search --class "$class" | sort -u)
}

function handle_term() {
  for pid in ${pids_to_kill[*]}; do
    echo "Killing $pid children..."
    for child_pid in "$(ps -o pid --no-headers --ppid ${pid})"; do
      if [[ -z $child_pid ]]; then
        continue
      fi
      echo "Killing child $child_pid"
      kill -9 $child_pid
    done
    echo "Killing $pid"
    kill -9 $pid
  done
}

# function wait_term() {
#   if [[ "${kill_needed}" == 1 ]]; then
#     for pid in ${pids_to_kill[*]}; do
#       kill $pid 2> /dev/null
#     done
#   fi
#   trap - TERM INT
#   for pid in ${pids_to_kill[*]}; do
#     wait $pid
#   done
# }

if [[ $run_left == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=left_eye ./run_libopendrop.sh \
    -B binaries/libopendrop_latest_ct_eyes_signals_optional \
    -s $source \
    --nodraw_signal_viewer \
    --control_state=$(pwd)/configs/interactive_eye_left.textproto \
    --control_port=9955 \
    --inject &
  pids_to_kill+="$! "
fi

if [[ $run_right == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=right_eye ./run_libopendrop.sh \
    -B binaries/libopendrop_latest_ct_eyes_signals_optional \
    -s $source \
    --nodraw_signal_viewer \
    --control_state=$(pwd)/configs/interactive_eye_right.textproto \
    --control_port=9945 \
    --inject &
  pids_to_kill+="$! "
fi

if [[ $run_manual_controlled == 1 ]]; then
  SDL_VIDEO_X11_WMCLASS=left_main ./run_libopendrop.sh \
    -s $source \
    --control_port=9966 \
    --control_state=$(pwd)/configs/mpk_mini_config.textproto &
  pids_to_kill+="$! "
  SDL_VIDEO_X11_WMCLASS=right_main ./run_libopendrop.sh \
    -s $source \
    --control_port=9965 \
    --control_state=$(pwd)/configs/mpk_mini_config.textproto &
  pids_to_kill+="$! "
fi

if [[ $run_left == 1 ]] || [[ $run_right == 1 ]]; then
  ./binaries/control_sender.par --input_filter='.*Feather.*' --ports=9955 --ports=9945 &
  pids_to_kill+="$! "
fi
if [[ $run_left == 1 ]] || [[ $run_right == 1 ]] || [[ $run_manual_controlled == 1 ]]; then
  ./binaries/control_sender.par --input_filter='.*MPK.*' --ports=9944 --ports=9966 --ports=9965 --ports=9988 &
  pids_to_kill+="$! "
fi

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
  while [[ -z $(xdotool search --class "left_main") ]]; do
    echo "Waiting for left_main"
    sleep 1
  done
  while [[ -z $(xdotool search --class "right_main") ]]; do
    echo "Waiting for right_main"
    sleep 1
  done
fi

sleep 2

if [[ $run_left == 1 ]]; then
  LEFT_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "left_eye")
fi

if [[ $run_right == 1 ]]; then
  RIGHT_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "right_eye")
fi

if [[ $run_manual_controlled == 1 ]]; then
  MANUAL_L_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "left_main")
  MANUAL_R_WINDOW_ID=$(find_window "OpenDrop Visualizer View" "right_main")
fi

echo $LEFT_WINDOW_ID $RIGHT_WINDOW_ID $MANUAL_WINDOW_ID

if [[ $run_manual_controlled == 1 ]]; then
  i3-msg "[id=\"$MANUAL_L_WINDOW_ID\"] move container to output DP-0.1"
  i3-msg "[id=\"$MANUAL_L_WINDOW_ID\"] fullscreen enable"
  i3-msg "[id=\"$MANUAL_R_WINDOW_ID\"] move container to output DP-0.2"
  i3-msg "[id=\"$MANUAL_R_WINDOW_ID\"] fullscreen enable"
fi

if [[ $run_left == 1 ]]; then
  sleep 2
  i3-msg "[id=\"$LEFT_WINDOW_ID\"] move container to output DP-0.1"
  i3-msg "[id=\"$LEFT_WINDOW_ID\"] fullscreen enable"
fi

if [[ $run_right == 1 ]]; then
  sleep 2
  i3-msg "[id=\"$RIGHT_WINDOW_ID\"] move container to output DP-0.2"
  i3-msg "[id=\"$RIGHT_WINDOW_ID\"] fullscreen enable"
fi

echo "Pids to kill: ${pids_to_kill[@]}"
trap 'handle_term' TERM INT

for pid in ${pids_to_kill[*]}; do
  wait $pid
done
#wait_term

echo "Done, exiting"
