# Rules for the test code (use `make check` to execute)
TESTS = RCPTests
check_PROGRAMS = $(TESTS)
RCPTests_SOURCES = \
	../SAM7s_base/util/mod_string.h \
	../SAM7s_base/util/mod_string.c \
	../SAM7s_base/util/modp_numtoa.h \
	../SAM7s_base/util/modp_numtoa.c \
	../logger/loggerConfig.h \
	../logger/loggerConfig.c \
	numtoa_test.h \
	numtoa_test.cpp \
	RCPTest.cpp 
RCPTests_CXXFLAGS = $(CPPUNIT_CFLAGS) 
RCPTests_CPPFLAGS =  -std=gnu99 -I../SAM7s_base/util -I../logger -DRCP_TESTING
RCPTests_LDFLAGS = -ldl -lcppunit 