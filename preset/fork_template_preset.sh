#!/bin/bash

# This script forks `template_preset` into a new preset. The script takes a
# single argument, which is a snake_case name for the new preset.

function echoerr() {
  echo -e "$@" 1>&2
}

function usage() {
  echoerr "USAGE:"
  echoerr ""
  echoerr "$0 [-t template_name] camel_case_name"
  echoerr ""
  echoerr "Args:"
  echoerr "  template_name:    Name of preset to fork, in camel_case (matches"
  echoerr "                    regex \`^[a-z][0-9a-z_]*$\`). Defaults to "
  echoerr "                    \"template_preset\"."
  echoerr "  camel_case_name:  Name of forked preset, in camel_case (matches"
  echoerr "                    regex \`^[a-z][0-9a-z_]*$\`)."
}

function errexit() {
  echoerr $@
  usage
  exit 1
}

function screaming_snake_case() {
  local name_snake_case=$1

  echo "${name_snake_case}" | tr [:lower:] [:upper:]
}

function camel_case() {
  local name_snake_case=$1

  echo "${name_snake_case}" | sed 's/\(^[a-z]\|_[a-z]\)/\U\1/g' | sed 's/_//g'
}

function recursive_replace() {
  local directory=$1
  local search_term=$2
  local replace_term=$3

  grep -rl "${search_term}" "${directory}" |
    xargs -I {} sed "s/${search_term}/${replace_term}/g" -i {}
}

function check_snake_case() {
  local name_snake_case=$1
  if [[ -z $(echo "${name_snake_case}" | grep -e '^[a-z][0-9a-z_]*$') ]]; then
    errexit "Input name ${name_snake_case} is not written in snake_case"
  fi
}

template_name_snake_case="template_preset"

while [[ ! -z "$@" ]]; do
  arg=$1
  shift

  case $arg in
    '-t')
      template_name_snake_case=$1
      shift
      ;;

    *)
      if [[ ! -z "${name_snake_case}" ]]; then
        errexit "Unknown argument: \"$1\""
      fi
      name_snake_case=$arg
      ;;
  esac
done

check_snake_case "${name_snake_case}"
if [[ ! -z "${template_name_snake_case}" ]]; then
  check_snake_case "${template_name_snake_case}"
fi

preset_dir="$(dirname $0)"
name_camel_case=$(camel_case ${name_snake_case})
name_screaming_snake_case=$(screaming_snake_case ${name_snake_case})

template_name_camel_case=$(camel_case ${template_name_snake_case})
template_name_screaming_snake_case=$(screaming_snake_case ${template_name_snake_case})

echo "Forking \"${preset_dir}/${template_name_snake_case}\" ==> \"${preset_dir}/${name_snake_case}\"."

cp -r "${preset_dir}/${template_name_snake_case}" "${preset_dir}/${name_snake_case}"
recursive_replace "${preset_dir}/${name_snake_case}/" "${template_name_screaming_snake_case}" \
  "${name_screaming_snake_case}"
recursive_replace "${preset_dir}/${name_snake_case}/" "${template_name_snake_case}" \
  "${name_snake_case}"
recursive_replace "${preset_dir}/${name_snake_case}/" "${template_name_camel_case}" \
  "${name_camel_case}"
mv "${preset_dir}/${name_snake_case}/${template_name_snake_case}.cc" \
  "${preset_dir}/${name_snake_case}/${name_snake_case}.cc"
mv "${preset_dir}/${name_snake_case}/${template_name_snake_case}.h" \
  "${preset_dir}/${name_snake_case}/${name_snake_case}.h"
