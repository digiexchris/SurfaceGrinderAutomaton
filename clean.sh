#/bin/bash

rm -Rf build
mkdir -p build
cd build
cmake --preset default ../ 