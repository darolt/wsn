#!/bin/bash

echo 'from directory ./python:' >> cat_all.txt
for file in $(find ./python -name '*.py'); do
  echo '-- file: ' + $file >> cat_all.txt
  cat $file >> cat_all.txt
done

echo 'from directory ./cc:' >> cat_all.txt
for file in $(find ./cc -name '*.h' -or -name '.cc' -or -name '.i'); do
  echo '-- file: ' + $file >> cat_all.txt
  cat $file >> cat_all.txt
done
