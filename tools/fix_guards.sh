#!/bin/bash

if [[ ! "$(basename $(pwd))" == 'libopendrop' ]]; then
  echo "Must be run from project root directory!"
  exit 1
fi

bazelisk run @fix_guards//:fix -- \
  --root ~+ --paths="$(find ~+ -type f -name '*.h')"
