#!/bin/sh

aclocal
autoheader
autoconf
automake -a
./configure
#make

