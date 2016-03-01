MAJOR  := 2
MINOR  := 9
BUGFIX := 0
API    := 1

VERSION_STR := "$(MAJOR).$(MINOR).$(BUGFIX)"
#
# Gets the sha-1 hash (hex encoded string) of our build)
#
GIT_DESCRIPTION := "\"$(shell git describe --dirty)\""

#
# Build types:
#
# * RELEASE_TYPE_DEVEL    - (Default) Build from a dev machine.
# * RELEASE_TYPE_OFFICIAL - Official build from our build server.
#                           Implies a tag is associated with this
#                           release.
# * RELEASE_TYPE_BETA     - A beta release.  One designed for user
#                           testing, but has no official support.
#
RELEASE_TYPE := RELEASE_TYPE_DEVEL

ifeq ($(RELEASE),BETA)
RELEASE_TYPE := RELEASE_TYPE_BETA
endif

#TAG := $(shell git describe --exact-match HEAD || true)
#ifneq ($(TAG),)
#RELEASE_TYPE := RELEASE_TYPE_OFFICIAL
#endif

VERSION_CFLAGS := \
-DAPI_REV=$(API) \
-DMAJOR_REV=$(MAJOR) \
-DMINOR_REV=$(MINOR) \
-DBUGFIX_REV=$(BUGFIX) \
-DRC_BUILD_GIT_DESCRIPTION=$(GIT_DESCRIPTION) \
-DRC_BUILD_RELEASE_TYPE=$(RELEASE_TYPE) \
