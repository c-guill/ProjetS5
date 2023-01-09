#!/bin/bash
for file in *
do
  ext="${file##*.}"
  if [[ $ext == c && $file != eabi-none-lib.c ]]
  then
    arm-none-eabi-gcc -c -mbig-endian -mno-thumb-interwork eabi-none-lib.c $file
  fi
done