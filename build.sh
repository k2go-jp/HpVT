#!/bin/sh

set -e

case $1 in
  clean)
    arg=clean
    rm -rf ./bin/
    ;;
  *)
    arg=all
    if [ ! -d bin ]; then mkdir bin; fi
    ;;
esac

BINARY_NAME=hpvt

cd src/
echo "Enter Directory src/"
make -j4 $arg
cd ..

if [ -d ./bin/ ]; then
  echo "Copy Binary to bin/"
  if [ -f src/${BINARY_NAME} ]; then
    cp -f src/${BINARY_NAME} ./bin/;
    strip -g ./bin/${BINARY_NAME}
  fi
fi
