#!/bin/bash
if [ $# -ne 2 ]
then
  echo 'usage: ./test.sh nom_dossier_source nom_dossier_tests'
  exit 1
fi
h=$1/getELFHeader
S=$1/getSectionHeaderTable
x=$1/getSection
s=$1/getSymbolTable
r=$1/getRelocationTable
#diff -q <(./$h elf/Examples_fusion/file1.o) <(readelf -h elf/Examples_fusion/file1.o)
for file in "$2"*
do
  ext="${file##*.}"
  if [[ $ext == o ]]
  then
    echo "Test sur le fichier $file:"
    echo "option: -h"
    diff -q <(./$h $file) <(readelf -h $file)
    echo "option: -S"
    diff -w <(./$S $file) <(readelf -S $file)
#    echo "option: -x"
#    diff -q <(./$x $file) <(readelf -x $file)
#    echo "option: -s"
#    diff -w <(./$s $file) <(readelf -s $file)
#    echo "option: -r"
#    diff -w <(./$r $file) <(readelf -r $file)

  fi
done