#!/bin/bash

# This script forks `template_preset` into a new preset. The script takes a
# single argument, which is a snake_case name for the new preset.

function echoerr() {
  echo -e "$@" 1>&2
}

function usage() {
  echoerr "USAGE:"
  echoerr ""
  echoerr "$0 camel_case_name"
  echoerr ""
  echoerr "Args:"
  echoerr "  camel_case_name:  Name of forked preset, in camel_case (matches"
  echoerr "                    regex \`^[a-z][a-z_]*$\`)"
}

function errexit() {
  echoerr $@
  usage
  exit 1
}

function screaming_snake_case() {
  name_snake_case=$1

  echo "${name_snake_case}" | tr [:lower:] [:upper:]
}

function camel_case() {
  name_snake_case=$1

  echo "${name_snake_case}" | sed 's/\(^[a-z]\|_[a-z]\)/\U\1/g' | sed 's/_//g'
}

function recursive_replace() {
  directory=$1
  search_term=$2
  replace_term=$3

  grep -rl "${search_term}" "${directory}" |
    xargs -I {} sed "s/${search_term}/${replace_term}/g" -i {}
}

name_snake_case=$1

if [[ $# != 1 ]]; then
  errexit "$0 takes exactly 1 argument"
fi

if [[ -z $(echo "${name_snake_case}" | grep -e '^[a-z][a-z_]*$') ]]; then
  errexit "Input name ${name_snake_case} is not written in snake_case"
fi

name_camel_case=$(camel_case ${name_snake_case})
name_screaming_snake_case=$(screaming_snake_case ${name_snake_case})

cp -r template_preset "${name_snake_case}"
recursive_replace "${name_snake_case}/" "TEMPLATE_PRESET" \
  "${name_screaming_snake_case}"
recursive_replace "${name_snake_case}/" "template_preset" "${name_snake_case}"
recursive_replace "${name_snake_case}/" "TemplatePreset" "${name_camel_case}"
mv "${name_snake_case}/template_preset.cc" \
  "${name_snake_case}/${name_snake_case}.cc"
mv "${name_snake_case}/template_preset.h" \
  "${name_snake_case}/${name_snake_case}.h"
