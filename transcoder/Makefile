#Copyright (C) 2024 by Thomas A. Early, N7TAE

include ../urfd.mk

GCC = g++

ifeq ($(debug), true)
CFLAGS = -ggdb3 -W -Werror -Icodec2 -Iimbe -I../common -MMD -MD -std=c++17
else
CFLAGS = -W -Werror -Icodec2 -Iimbe -I../common -MMD -MD -std=c++17
endif

LDFLAGS = -lftd2xx -pthread

SRCS = $(wildcard *.cpp) $(wildcard ../common/*.cpp) $(wildcard imbe/*.cpp) $(wildcard codec2/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
EXE = tcd

.PHONY : all
all : $(EXE)

$(EXE) : $(OBJS)
	$(GCC) $(OBJS) $(LDFLAGS) -o $@

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $< -o $@

.PHONY : clean
clean :
	$(RM) $(EXE) *.d *.o ../common/*.d ../common/*.o imbe/*.o imbe/*.d codec2/*.o codec2/*.d

-include $(DEPS)
