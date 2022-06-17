#!/bin/bash

mkdir -p build
cd build
sudo cmake --no-warn-unused-cli -DCMAKE_TOOLCHAIN_FILE:STRING=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE:STRING=Debug -S .. -B . -G "Unix Makefiles"
sudo make

