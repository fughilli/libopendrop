#!/bin/bash

set -o errexit
set -o pipefail

port=3333

libopendrop_pid=$(ps aux | grep 'libopendrop\ ' | awk '{print $2}')
if [[ -z $libopendrop_pid ]]; then
  echo "No running instance of libopendrop"
  exit 1
fi
gdbserver --attach localhost:$port $libopendrop_pid
