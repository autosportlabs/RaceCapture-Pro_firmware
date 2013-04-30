# Rules for the test code (use `make check` to execute)
TESTS = RCPTests
check_PROGRAMS = $(TESTS)
RCPTests_SOURCES = numtoa_test.h numtoa_test.cpp RCPTest.cpp
RCPTests_CXXFLAGS = $(CPPUNIT_CFLAGS)
RCPTests_LDFLAGS = -ldl -lcppunit