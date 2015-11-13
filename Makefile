# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

CXX ?= g++
YAML_CPP_INCLUDE_PATH ?= ./yaml-cpp/include
CXXFLAGS += -g --std=c++11 -fPIC -Wall -I$(YAML_CPP_INCLUDE_PATH)
ENABLE_ASAN ?= false
GDB ?= gdb
CONFIGS := $(shell find conf -name "*.yaml")
SRC := $(shell find src -name "*.cc")
OBJS := $(patsubst %.cc, %.o, $(SRC))
OUTDIR := output

ifeq ($(ENABLE_ASAN), true)
	CXXFLAGS += -fsanitize=address
endif

.PHONY: all clean run gdb lldb test php js lines

-include Makefile.local

all: main $(OUTDIR) $(OUTDIR)/configuration.cc $(OUTDIR)/configuration.h $(OUTDIR)/configuration-json.cc $(OUTDIR)/configuration-json.h $(OUTDIR)/configuration.js $(OUTDIR)/configuration.php

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/configuration.cc: main
	./$< $(CONFIGS) --cpp-code > $(OUTDIR)/configuration.cc

$(OUTDIR)/configuration.h: main
	./$< $(CONFIGS) --cpp-header > $(OUTDIR)/configuration.h

$(OUTDIR)/configuration-json.cc: main
	./$< $(CONFIGS) --cpp-json-code > $(OUTDIR)/configuration-json.cc

$(OUTDIR)/configuration-json.h: main
	./$< $(CONFIGS) --cpp-json-header > $(OUTDIR)/configuration-json.h

$(OUTDIR)/configuration.js: main
	./$< $(CONFIGS) --js > $(OUTDIR)/configuration.js

$(OUTDIR)/configuration.php: main
	./$< $(CONFIGS) --php > $(OUTDIR)/configuration.php

$(OBJS): $(SRC)
	cd src && make

run: main
	./$< $(CONFIGS);

gdb: main
	$(GDB) -ex run -ex quit --args ./$< $(CONFIGS);

lldb: main
	lldb -s ./lldb -- ./$< $(CONFIGS);

main: $(OBJS) yaml-cpp/libyaml-cpp.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

test: php js

php: main tests/test1.php $(CONFIGS)
	(./$< --php $(CONFIGS); cat tests/test1.php) | php > /dev/null

js: main tests/test1.js $(CONFIGS)
	(./$< --js $(CONFIGS); cat tests/test1.js) | node > /dev/null

yaml-cpp/include/yaml-cpp/yaml.h yaml-cpp/CMakeLists.txt dep:
	git submodule update --init $<;

yaml-cpp/Makefile: yaml-cpp/CMakeLists.txt
	[ -r $@ ] || ( cd $(dir $<) && cmake .; );

yaml-cpp/libyaml-cpp.a: yaml-cpp/Makefile
	[ -r $@ ] || make -C $(dir $<);

lines:
	wc -l src/*.cc include/*.h;

clean:
	@cd src && make clean;
	@rm -fr main output;
