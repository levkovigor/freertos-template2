#!/bin/sh
# Execute this in the CMake build folder to build and package Coverity analysis data
cov-build --dir cov-int cmake --build . -j && tar czvf sourceobsw-coverity.tgz cov-int