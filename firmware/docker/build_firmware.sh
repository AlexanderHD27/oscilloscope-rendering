#!/usr/bin/bash

cd ./firmware/build
rm -rf *

export PICO_SDK_PATH_OVERRIDE="/data/firmware/lib/pico-sdk"
echo $PICO_SDK_PATH_OVERRIDE

cmake ..
cmake --build . --config Debug
make -j4 all