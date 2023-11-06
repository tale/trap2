#!/usr/bin/env bash

# Replace glibtool with libtool in subprojects/vterm/Makefile
sed -i '' -e 's/glibtool/libtool/g' subprojects/vterm/Makefile

git submodule update --init --recursive
make -j8 -C subprojects/vterm
