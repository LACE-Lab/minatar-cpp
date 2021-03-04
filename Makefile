INC = $(shell python3-config --includes)
LIB = $(shell python3-config --embed --ldflags)
OPTS = -g -Wall -std=c++1z

MinAtarInterface.o: MinAtarInterface.cpp MinAtarInterface.hpp CPyObject.hpp
	g++ ${OPTS} ${INC} -c MinAtarInterface.cpp
