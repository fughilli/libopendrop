#!/bin/bash

source=bluetooth

./run_libopendrop.sh -s $source --control_state=$(pwd)/configs/interactive_eye_left.textproto --control_port=9955 --inject &
./run_libopendrop.sh -s $source --control_state=$(pwd)/configs/interactive_eye_right.textproto --control_port=9945 --inject &
./run_libopendrop.sh -b -s $source --control_state=$(pwd)/configs/mpk_mini_config.textproto &
bin_pid=$!

bazelisk run //debug:control_sender -- --input_filter='.*Feather.*' --ports=9955 --ports=9945 &
bazelisk run //debug:control_sender -- --input_filter='.*MPK.*' --ports=9944

kill $bin_pid
killall libopendrop_for_ct_live
