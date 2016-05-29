#!/bin/bash

cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4 VERBOSE=1
