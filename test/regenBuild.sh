#!/bin/sh
#the below path might need to be adjusted for your platform. this is for Fedora
aclocal -I /usr/local/share/aclocal
autoconf
automake -a
./configure
