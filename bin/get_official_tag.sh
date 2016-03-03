#!/bin/bash
#
# Simple script that will echo the official annotated tag if this
# an official release.  Else it will reply with nothing.  Always
# exit with 0 status because otherwise Make will die if it isn't
# an official release.
#
git describe --exact-match HEAD 2>/dev/null
exit 0
