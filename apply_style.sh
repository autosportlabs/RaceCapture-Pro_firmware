#!/bin/sh
astyle --style=linux --indent=spaces=8 --recursive "include/*.h"
astyle --style=linux --indent=spaces=8 --recursive "src/*.c"
astyle --style=linux --indent=spaces=8 --recursive "test/*.h"
astyle --style=linux --indent=spaces=8 --recursive "test/*.c"
astyle --style=linux --indent=spaces=8 "*.h"
astyle --style=linux --indent=spaces=8 "*.c"
