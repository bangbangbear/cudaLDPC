CPP = g++
CC = gcc
NVCC = nvcc

CFLAGS= -Wall

ifeq ($(VER), debug)
CFLAGS+= -ggdb
else
CFLAGS+= -O3
endif

CPPFLAGS= -std=c++11
NVCCFLAGS= -arch sm_61

LIBFLAGS= -lcudart -L/opt/cuda/lib64

ROOTDIR=$(shell echo $(CURDIR))
SRCDIR=$(ROOTDIR)/src
VPATH=${SRCDIR}

SRC=$(wildcard ${SRCDIR}/*.cpp)
OBJ=$(notdir $(SRC:.cpp=.cpp.o))

CUSRC=$(wildcard ${SRCDIR}/*.cu)
CUOBJ=$(notdir $(CUSRC:.cu=.cu.o))

INCLUDE= -I${SRCDIR} -I/opt/cuda/include/

dec: $(CUOBJ) $(OBJ) 
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(LIBFLAGS) -o $@  $^

%.cpp.o: %.cpp
	@echo C++ -c -o $@ 
	@$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<

%.cu.o: %.cu
	@echo C++ -c -o $@ 
	@$(NVCC) $(NVCCFLAGS) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.c
	@echo CC -c -o $@ 
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

.PHONY: clean
clean:
	@rm -f dec
#	@rm -f *.so
	@rm -f $(OBJ) $(CUOBJ)
	@find $(ROOTDIR)/src -name "*.d" -exec rm {} \;
	@find $(ROOTDIR)/src -name "*.d.*" -exec rm {} \;
	@echo Directory cleaned up. 

include $(SRC:.cpp=.d)
%.d: %.cpp
	@set -e; rm -f $@; \
	g++ -MM $(CPPFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

