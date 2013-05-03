#-----Macros---------------------------------

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
	$(CXX) $(CXXFLAGS) $< -o $@ -lm $(TLIB) -lg++

#-----File Dependencies----------------------

SRC = 	loggerData_test.cpp \
	numtoa_test.cpp \
	RCPTest.cpp

OBJ = $(addsuffix .o, $(basename $(SRC)))

usepix: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) -lm $(TLIB) -lg++

#-----Other stuff----------------------------
depend:
	makedepend $(CXXFLAGS) -Y $(SRC)

clean:
	rm -f $(OBJ)
