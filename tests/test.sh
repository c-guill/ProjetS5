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
for file in "$2"*
do
  ext="${file##*.}"
  if [[ $ext == o ]]
  then
    echo "Test sur le fichier $file:"
    echo "option: -h "
    diff <(./$h $file) <(readelf -h $file)
    echo "option: -S"
    diff -a -w  --color <(./$S $file) <(readelf -S $file)
    echo "option: -x"
    i=0
    v=$($x $i $file)
    v2=$( readelf -x $i $file )
    while [[ $v != "" && $v2 != "" ]]
    do
    diff -w --color <(echo $v) <(echo $v2)
    i=$( expr $i + 1 )
    v=$($x $i $file)
    v2=$( readelf -x $i $file )
    done
    echo "option: -s"
    diff -w --color <(./$s $file) <(readelf -s $file)
    echo "option: -r"
    diff -w --color <(./$r $file) <(readelf -r $file)

  fi
done