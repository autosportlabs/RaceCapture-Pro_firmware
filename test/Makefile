#-----Macros---------------------------------

include ../version.mk

NAME=rcptest
SIMNAME = rcpsim

RCP_BASE=..
RCP_SRC=$(RCP_BASE)/src
RCP_INC=$(RCP_BASE)/include
SAM7S_SRC=$(RCP_BASE)/SAM7s_base
MESSAGING_SRC=$(RCP_SRC)/messaging
COMMAND_SRC=$(RCP_SRC)/command
MOCK_DIR=logger_mock
GPS_DIR=gps
UTIL_DIR=util
BUILD_DIR=build

INCLUDES = 	-I. \
		-I./include \
		-I$(RCP_BASE) \
		-I$(RCP_BASE)/logger \
		-I$(SAM7S_SRC)/fat_sd_at91 \
		-I$(RCP_INC) \
		-I$(RCP_INC)/logging \
		-I$(RCP_INC)/gps \
		-I$(RCP_INC)/jsmn \
		-I$(RCP_INC)/api \
		-I$(RCP_INC)/filter \
		-I$(RCP_INC)/logger \
		-I$(RCP_INC)/channels \
		-I$(RCP_INC)/virtual_channel \
		-I$(RCP_INC)/tracks \
		-I$(RCP_INC)/messaging \
		-I$(RCP_INC)/lua \
		-I$(RCP_INC)/command \
		-I$(RCP_INC)/predictive_timer \
		-I$(RCP_INC)/auto_config \
		-I$(RCP_INC)/util \
		-I$(RCP_INC)/serial \
		-I$(SAM7S_SRC)/lua \
		-I$(SAM7S_SRC)/command \
		-I$(SAM7S_SRC)/uart \
		-I$(SAM7S_SRC)/usb/include \
		-I$(RCP_INC) \
		-I$(RCP_INC)/usart \
		-I$(RCP_INC)/usb_comm \
		-I$(RCP_INC)/imu \
		-I$(RCP_INC)/ADC \
		-I$(RCP_INC)/timer \
		-I$(RCP_INC)/PWM \
		-I$(RCP_INC)/LED \
		-I$(RCP_INC)/cpu \
		-I$(RCP_INC)/CAN \
		-I$(RCP_INC)/OBD2 \
		-I$(RCP_INC)/GPIO \
		-I$(RCP_INC)/watchdog \
		-I$(RCP_INC)/memory \
		-I$(RCP_INC)/sdcard \
		-I$(MOCK_DIR) \
		-I$(GPS_DIR) \
		-I$(UTIL_DIR)

# set up compiler and options
CXX = g++
CC = g++
CXXFLAGS = -m32 -g $(INCLUDES) -DRCP_TESTING -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX)
CPPFLAGS = -m32 -g $(INCLUDES) -DRCP_TESTING -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX)

#-----Suffix Rules---------------------------
# set up C++ suffixes and relationship between .cc and .o files

dir_guard=@mkdir -p $(@D)

build/%.o: %.c
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: %.cpp
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/rcp_base/%.o: ../%.c
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#-----File Dependencies----------------------

T_SRC = loggerApi_test.cpp \
		date_time_test.cpp \
		launch_control_test.cpp \
		loggerConfig_test.cpp \
		sampleRecord_test.cpp \
		PredictiveTimeTest2.cpp \
		sector_test.cpp \
		track_test.cpp \
		loggerData_test.cpp \
		$(GPS_DIR)/gps_test.cpp \
		$(UTIL_DIR)/numtoa_test.cpp \
		$(UTIL_DIR)/atonum_test.cpp

SRC = 	mock_uart.c \
		mock_usb_comm.c \
		mock_serial.c \
		src/system/task.cpp \
		$(MOCK_DIR)/util/taskUtil_mock.c \
		$(RCP_SRC)/launch_control.c \
		$(RCP_SRC)/lua/luaScript.c \
		$(RCP_SRC)/util/modp_numtoa.c \
		$(RCP_SRC)/util/modp_atonum.c \
		$(RCP_SRC)/util/mod_string.c \
		$(RCP_SRC)/jsmn/jsmn.c \
		$(RCP_SRC)/api/api.c \
		$(RCP_SRC)/OBD2/OBD2.c \
		$(RCP_SRC)/logging/printk.c \
		$(RCP_SRC)/logging/ring_buffer.c \
		$(RCP_SRC)/logger/loggerApi.c \
		$(RCP_SRC)/filter/filter.c \
		$(RCP_SRC)/imu/imu.c \
		$(RCP_SRC)/ADC/ADC.c \
		$(RCP_SRC)/memory/memory.c \
		$(RCP_SRC)/timer/timer.c \
		$(RCP_SRC)/PWM/PWM.c \
		$(RCP_SRC)/LED/LED.c \
		$(RCP_SRC)/cpu/cpu.c \
		$(RCP_SRC)/usart/usart.c \
		$(RCP_SRC)/serial/serial.c \
		$(RCP_SRC)/GPIO/GPIO.c \
		$(RCP_SRC)/watchdog/watchdog.c \
		$(RCP_SRC)/CAN/CAN.c \
		$(MOCK_DIR)/imu_device_mock.c \
		$(MOCK_DIR)/timer_device_mock.c \
		$(MOCK_DIR)/ADC_device_mock.c \
		$(MOCK_DIR)/cpu_device_mock.c \
		$(MOCK_DIR)/PWM_device_mock.c \
		$(MOCK_DIR)/LED_device_mock.c \
		$(MOCK_DIR)/luaTask_mock.c \
		$(MOCK_DIR)/GPIO_device_mock.c \
		$(MOCK_DIR)/memory_device_mock.c \
		$(MOCK_DIR)/loggerNotifications_mock.c \
		$(MOCK_DIR)/sdcard_mock.c \
		$(MOCK_DIR)/watchdog_device_mock.c \
		$(MOCK_DIR)/CAN_device_mock.c \
		$(RCP_SRC)/predictive_timer/predictive_timer_2.c \
		$(RCP_SRC)/auto_config/auto_track.c \
		$(RCP_SRC)/util/linear_interpolate.c \
		$(RCP_SRC)/logger/loggerConfig.c \
		$(RCP_SRC)/virtual_channel/virtual_channel.c \
		$(RCP_SRC)/tracks/tracks.c \
		$(RCP_SRC)/gps/gps.c \
		$(RCP_SRC)/gps/dateTime.c \
		$(RCP_SRC)/gps/geopoint.c \
		$(RCP_SRC)/gps/geoCircle.c \
		$(RCP_SRC)/logger/sampleRecord.c \
		$(RCP_SRC)/logger/loggerSampleData.c \
		$(RCP_SRC)/logger/loggerData.c \
		$(RCP_SRC)/logger/loggerHardware.c \
		$(RCP_SRC)/channels/channelMeta.c \

#		$(RCP_SRC)/logger/loggerTaskEx.c \


OBJ_TEST = $(addprefix build/, $(addsuffix .o, $(subst $(RCP_BASE)/, rcp_base/, $(basename $(SRC) $(T_SRC) RCPTest.cpp))))
OBJ_SIM = $(addprefix build/, $(addsuffix .o, $(subst $(RCP_BASE)/, rcp_base/, $(basename $(SRC) RCPSim.cpp))))

all: test sim

test: $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ_TEST) -lm -lcppunit

sim: $(OBJ_SIM)
	$(CXX) $(CXXFLAGS) -o $(SIMNAME) $(OBJ_SIM) -lm

clean:
	rm -f $(OBJ_TEST) $(OBJ_SIM) $(NAME) $(SIMNAME)
