#-----Macros---------------------------------

include ../version.mk

NAME=rcptest

RCP_BASE=..
RCP_SRC=$(RCP_BASE)/src
RCP_INC=$(RCP_BASE)/include
SAM7S_SRC=$(RCP_BASE)/SAM7s_base
MESSAGING_SRC=$(RCP_SRC)/messaging
COMMAND_SRC=$(SAM7S_SRC)/command
MOCK_DIR=./logger_mock
GPS_DIR=./gps
UTIL_DIR=./util

INCLUDES = 	-I. \
		-I$(RCP_BASE) \
		-I$(RCP_INC)/util \
		-I$(RCP_BASE)/logger \
		-I$(RCP_INC)/logging \
		-I$(RCP_INC)/gps \
		-I$(RCP_INC)/jsmn \
		-I$(RCP_INC)/api \
		-I$(RCP_INC)/filter \
		-I$(RCP_INC)/logger \
		-I$(RCP_INC)/messaging \
		-I$(RCP_INC)/predictive_timer \
		-I$(RCP_INC)/util \
		-I$(SAM7S_SRC)/serial \
		-I$(SAM7S_SRC)/lua \
		-I$(SAM7S_SRC)/command \
		-I$(RCP_INC) \
		-I$(SAM7S_SRC)/uart \
		-I$(SAM7S_SRC)/usb/include \
		-I$(RCP_INC)/accelerometer \
		-I$(MOCK_DIR) \
		-I$(GPS_DIR) \
		-I$(UTIL_DIR)

# set up compiler and options
CXX = g++
CC = g++
CXXFLAGS = -m32 -g $(INCLUDES) -DRCP_TESTING -DMAJOR_REV=\"$(MAJOR)\" -DMINOR_REV=\"$(MINOR)\" -DBUGFIX_REV=\"$(BUGFIX)\"
CPPFLAGS = -m32 -g $(INCLUDES) -DRCP_TESTING -DMAJOR_REV=\"$(MAJOR)\" -DMINOR_REV=\"$(MINOR)\" -DBUGFIX_REV=\"$(BUGFIX)\"

#-----Suffix Rules---------------------------
# set up C++ suffixes and relationship between .cc and .o files

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

.cc :
	$(CXX) $(CXXFLAGS) $< -o $@ -lm -lcppunit

#-----File Dependencies----------------------

SRC = 	loggerData_test.cpp \
		$(GPS_DIR)/gps_test.cpp \
		$(UTIL_DIR)/numtoa_test.cpp \
		$(UTIL_DIR)/atonum_test.cpp \
		loggerApi_test.cpp \
		loggerConfig_test.cpp \
		predictiveTime_test.cpp \
		sampleRecord_test.cpp \
		RCPTest.cpp \
		mock_uart.c \
		mock_usb_comm.c \
		mock_serial.c \
		$(RCP_SRC)/util/modp_numtoa.c \
		$(RCP_SRC)/util/modp_atonum.c \
		$(RCP_SRC)/jsmn/jsmn.c \
		$(RCP_SRC)/api/api.c \
		$(RCP_SRC)/logging/printk.c \
		$(RCP_SRC)/logging/ring_buffer.c \
		$(SAM7S_SRC)/serial/serial.c \
		$(RCP_SRC)/logger/loggerApi.c \
		$(RCP_SRC)/filter/filter.c \
		$(RCP_SRC)/accelerometer/accelerometer.c \
		$(MOCK_DIR)/accelerometer_device_mock.c \
		$(RCP_SRC)/predictive_timer/predictive_timer.c \
		$(RCP_SRC)/util/linear_interpolate.c \
		$(RCP_BASE)/logger/loggerConfig.c \
		$(RCP_SRC)/gps/gps.c \
		$(RCP_SRC)/gps/geometry.c \
		$(RCP_BASE)/logger/sampleRecord.c \
		$(RCP_BASE)/logger/loggerSampleData.c \
		$(RCP_BASE)/logger/loggerData.c \
		$(MOCK_DIR)/loggerHardware.c 

OBJ = $(addsuffix .o, $(basename $(SRC)))

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) -lm -lcppunit

clean:
	rm -f $(OBJ) $(NAME)
