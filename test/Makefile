#-----Macros---------------------------------

NAME=rcptest

RCP_SRC=../SAM7s_base/util

INCLUDES = 	-I. \
		-I.. \
		-I../SAM7s_base/util \
		-I../logger \

# set up compiler and options
CXX = g++
CXXFLAGS = -g $(INCLUDES)

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
	$(RCP_SRC)/modp_numtoa.c 

OBJ = $(addsuffix .o, $(basename $(SRC)))

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) -lm -lcppunit

clean:
	rm -f $(OBJ) $(NAME)
