#!/bin/bash
for file in *
do
  ext="${file##*.}"
  if [[ $ext == c ]]
  then
    arm-none-eabi-gcc -c -mbig-endian -mno-thumb-interwork $file
  fi
done