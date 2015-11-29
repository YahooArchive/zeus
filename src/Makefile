# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

CXX ?= g++
YAML_CPP_PATH ?= ../yaml-cpp
CXXFLAGS += -g --std=c++11 -fPIC
CXXFLAGS += -I$(YAML_CPP_PATH)/include
CXXFLAGS += -Wall -Wno-deprecated-declarations
GDB ?= gdb
SRC := $(wildcard *.cc)
OBJS := $(patsubst %.cc, %.o, $(SRC))
BIN ?= zeus

ENABLE_ASAN ?= false
ifeq ($(ENABLE_ASAN), true)
	CXXFLAGS += -fsanitize=address
endif

.PHONY: all clean run lines

-include Makefile.local

all: $(BIN)

run: $(BIN)
	./$<;

$(BIN): $(OBJS) $(YAML_CPP_PATH)/libyaml-cpp.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

$(YAML_CPP_PATH)/libyaml-cpp.a:
	$(error Please make sure you have a static library at $@);

$(OBJS) : %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@;

lines:
	wc -l *.cc *.h;

clean:
	@rm -fv *.o $(BIN) 
