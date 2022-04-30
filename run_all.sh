#!/bin/bash

./run_libopendrop.sh -s bluetooth --control_state=$(pwd)/configs/interactive_eye_left.textproto --control_port=9955 --inject &
./run_libopendrop.sh -s bluetooth --control_state=$(pwd)/configs/interactive_eye_right.textproto --control_port=9945 --inject &

bazelisk run //debug:control_sender -- --input_filter='.*Feather.*' --ports=9955 --ports=9945
