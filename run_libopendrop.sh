#!/bin/bash

set -o errexit,pipefail

function echoerr() {
  echo $@ 1>&2
}

while [[ ! -z "$@" ]]; do
  arg=$1
  echoerr "Parsing arg: $arg"
  shift

  case $arg in
    '-s')
      source_type=$1
      shift
      ;;

    *)
      echo $1
      shift
      ;;
  esac
done

case $source_type in
  'bluetooth')
    SOURCE=$(pactl list sources | sed -n "s/\s\+Name.*\(bluez.*$\)/\1/p" |
      head -n 1)
    ;;

  'system')
    SOURCE=$(pactl list sources |
      sed -n "s/\s\+Name.*\(alsa_output.*monitor$\)/\1/p" | head -n 1)
    ;;

  *)
    echoerr "Unknown source type: $source"
    exit 1
    ;;
esac

bazelisk run //libopendrop:main -c opt -- \
  --pulseaudio_source="${SOURCE}" \
  --window_width=600 \
  --window_height=600 $@
