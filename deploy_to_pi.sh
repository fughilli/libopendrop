#!/bin/bash

set -o errexit
set -o pipefail

hostname=ledsuit

while [[ ! -z "$@" ]]; do
  arg=$1
  shift

  case $arg in
    '-t')
      hostname=$1
      shift
      ;;

    *)
      ;;
  esac
done

bazelisk build --distinct_host_configuration --config=pi :main -c dbg
cp ../bazel-bin/libopendrop/main /tmp/libopendrop
chmod +rw /tmp/libopendrop
scp /tmp/libopendrop "pi@${hostname}:~"
