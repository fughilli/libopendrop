#!/bin/bash

bazelisk build --distinct_host_configuration --config=pi :main -c dbg
cp ../bazel-bin/libopendrop/main /tmp/libopendrop
chmod +rw /tmp/libopendrop
scp /tmp/libopendrop pi@ledsuit:~
