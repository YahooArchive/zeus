# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

CXX ?= g++
YAML_CPP_INCLUDE_PATH ?= ../yaml-cpp/include
CXXFLAGS += -g --std=c++11 -fPIC -Wall -I$(YAML_CPP_INCLUDE_PATH) -I../include -Wno-deprecated-declarations
ENABLE_ASAN ?= false
GDB ?= gdb

SRC = \
	context.cc \
	cpp-code.cc \
	cpp-header.cc \
	cpp-json-code.cc \
	cpp-json-header.cc \
	dimension.cc \
	generator.cc \
	graph-builder.cc \
	graph-printer.cc \
	graph-trimmer.cc \
	graph-type-extractor.cc \
	graph-type-propagator.cc \
	graph.cc \
	ir.cc \
	java.cc \
	js.cc \
	key.cc \
	main.cc \
	parser.cc \
	php.cc \
	value.cc \
	structure.cc \
	structure-writer.cc \
	yaml.cc

OBJS = $(SRC:%.cc=%.o)

ifeq ($(ENABLE_ASAN), true)
	CXXFLAGS += -fsanitize=address
endif

-include Makefile.local

all: $(OBJS)

$(OBJS) : %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@;

clean:
	@rm -f *.o

