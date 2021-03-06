#!/bin/bash

set -o errexit
set -o pipefail

function echoerr() {
  echo -e $@ 1>&2
}

function usage() {
  echoerr "USAGE: $0 -s SOURCE [-d]"
  echoerr ""
  echoerr "Options:"
  echoerr "Flag\tDescription"
  echoerr "----\t-----------"
  echoerr "-s  \tPulseaudio source filter type. The first source of this type"
  echoerr "    \twill be selected. Source type should be one of [system, "
  echoerr "    \tbluetooth]."
  echoerr "-d  \tEnable debug logging."
  echoerr "-b  \tRun from the binary output of bazel instead of building "
  echoerr "    \tlibopendrop again."
  echoerr "-dv \tEnable debugging with valgrind. Implies '-b'."
  echoerr "-p  \tPosition on the screen. Must be of the form \`<x>,<y>\`."
}

enable_debug=0
run_binary=0
enable_valgrind=0

passthrough_args=()

while [[ ! -z "$@" ]]; do
  arg=$1
  shift

  case $arg in
    '-s')
      source_type=$1
      shift
      ;;

    '-d')
      enable_debug=1
      ;;

    '-b')
      run_binary=1
      ;;

    '-dv')
      run_binary=1
      enable_valgrind=1
      ;;

    '-p')
      position=$1
      shift
      ;;

    *)
      passthrough_args+="${arg}"
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

  'hdmi')
    SOURCE=$(pactl list sources |
      sed -n "s/\s\+Name.*\(alsa_output.*hdmi\-.*monitor$\)/\1/p" | head -n 1)
    ;;

  'microphone')
    SOURCE=$(pactl list sources |
      sed -n "s/\s\+Name.*\(alsa_input.*analog\-stereo$\)/\1/p" | head -n 1)
    ;;

  '')
    echoerr "Source type not provided!"
    usage
    exit 1
    ;;

  '?')
    echo "Supported source types: bluetooth system hdmi microphone"
    exit 0
    ;;

  *)
    echoerr "Unknown source type: $source"
    usage
    exit 1
    ;;
esac

if [[ $enable_debug == 1 ]]; then
  debug_options="-c dbg --copt=-ggdb --copt=-DENABLE_DEBUG_LOGGING"
else
  debug_options=""
fi

options="\
  --pulseaudio_source="${SOURCE}" \
  --window_width=600 \
  --window_height=600"

if [[ ! -z $position ]]; then
  match=$(echo $position | sed -n 's/^\(\-\?[0-9]\+,\-\?[0-9]\+\)$/\1/p')
  position_x=$(echo $position | sed -n 's/^\(\-\?[0-9]\+\),\-\?[0-9]\+$/\1/p')
  position_y=$(echo $position | sed -n 's/^\-\?[0-9]\+,\(\-\?[0-9]\+\)$/\1/p')
  if [[ -z $match ]]; then
    echoerr "Position does not match required format: $position"
    usage
    exit 1
  fi

  options="$options \
    --window_x=$position_x \
    --window_y=$position_y"
fi

if [[ $enable_valgrind == 1 ]]; then
  valgrind_command="valgrind --track-origins=yes --leak-check=full"
else
  valgrind_command=""
fi

if [[ $run_binary == 1 ]]; then
  $valgrind_command ../bazel-bin/libopendrop/main $options \
    ${passthrough_args[@]}
else
  bazelisk run //libopendrop:main $debug_options \
    --copt=-I/usr/include/SDL2 \
    -- \
    $options ${passthrough_args[@]}
fi
