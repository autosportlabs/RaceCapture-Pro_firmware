#-----Macros---------------------------------

NAME=rcptest

RCP_BASE=..
RCP_SRC=$(RCP_BASE)/src
RCP_INC=$(RCP_BASE)/include
SAM7S_SRC=$(RCP_BASE)/SAM7s_base
JSMN_SRC=$(RCP_SRC)/jsmn
API_SRC=$(RCP_SRC)/api


INCLUDES = 	-I. \
		-I$(RCP_BASE) \
		-I$(SAM7S_SRC)/util \
		-I$(RCP_BASE)/logger \
		-I$(RCP_INC)/jsmn \
		-I$(RCP_INC)/api \
		-I$(SAM7S_SRC)/serial \
		-I$(SAM7S_SRC)/lua \
		-I$(SAM7S_SRC)/command \
		-I$(RCP_INC) \
		-I$(SAM7S_SRC)/uart \
		-I$(SAM7S_SRC)/usb/include

# set up compiler and options
CXX = g++
CXXFLAGS = -g $(INCLUDES)
CPPFLAGS = -g $(INCLUDES) -std=c99

#-----Suffix Rules---------------------------
# set up C++ suffixes and relationship between .cc and .o files

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

.cc :
	$(CXX) $(CXXFLAGS) $< -o $@ -lm -lcppunit

#-----File Dependencies----------------------

SRC = 	loggerData_test.cpp \
	numtoa_test.cpp \
	RCPTest.cpp \
	uart_test.c \
	usb_comm_test.c \
	$(SAM7S_SRC)/util/modp_numtoa.c \
	$(JSMN_SRC)/jsmn.c \
	$(API_SRC)/api.c \
	$(RCP_BASE)/race_capture/printk.c \
	$(RCP_BASE)/race_capture/ring_buffer.c \
	$(SAM7S_SRC)/serial/serial.c 
	

OBJ = $(addsuffix .o, $(basename $(SRC)))

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) -lm -lcppunit

clean:
	rm -f $(OBJ) $(NAME)
