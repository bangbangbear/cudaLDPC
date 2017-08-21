CPP = nvcc
CC = gcc 

CFLAGS= -arch sm_61
CPPFLAGS= -std=c++11

ifeq ($(VER), debug)
CFLAGS+= -ggdb
else
CFLAGS+= -O3
endif

ROOTDIR=$(shell echo $(CURDIR))
SRCDIR=$(ROOTDIR)/src
VPATH=${SRCDIR}

SRC=$(wildcard ${SRCDIR}/*.cpp)
OBJ=$(notdir $(SRC:.cpp=.o))

INCLUDE= -I${SRCDIR} -I/opt/cuda/include/

dec: $(OBJ)
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(LIBFLAGS) -o $@  $^

%.o: %.cpp
	@echo C++ -c -o $@ 
	@$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.cu
	@echo C++ -c -o $@ 
	@$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.c
	@echo CC -c -o $@ 
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

.PHONY: clean
clean:
	@rm -f dec
#	@rm -f *.so
	@rm -f $(OBJ)
	@find $(ROOTDIR)/src -name "*.d" -exec rm {} \;
	@find $(ROOTDIR)/src -name "*.d.*" -exec rm {} \;
	@echo Directory cleaned up. 

include $(SRC:.cpp=.d)
%.d: %.cpp
	@set -e; rm -f $@; \
	g++ -MM $(CPPFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

