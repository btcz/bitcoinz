#!/bin/bash
HOST=x86_64-w64-mingw32
CXX=x86_64-w64-mingw32-g++-posix
CC=x86_64-w64-mingw32-gcc-posix

set -eu -o pipefail
set -x

HOST="${HOST}" CC="${CC}" CXX="${CXX}" ./zcutil/build.sh