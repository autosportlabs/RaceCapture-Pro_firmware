# Race Capture Firmware
#
# Copyright (C) 2016 Autosport Labs
#
# This file is part of the Race Capture firmware suite
#
# This is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License for more details. You should
# have received a copy of the GNU General Public License along with
# this code. If not, see <http://www.gnu.org/licenses/>.

#
# HOW TO USE...
#
# To use ccache with our builds, simply export the CCACHE variable in
# your shell environment.  It looks something like this:
#
# export CCACHE=ccache
#
# Doing this allows the developer to explicitly be able to use a
# program that speeds up compilation, but also forces the developer to
# realize that they are using it instead of doing it automaticlly.
# this will help save sanity should something go wrong with CCACHE.
#
# Users can always ensure that use of CCACHE is bypassed by simply
# passing CCACHE= as a make argument or by simply undefining it
# in their environment.
#

export CCACHE
