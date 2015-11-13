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

ifeq ($(ENABLE_ASAN), true)
	CXXFLAGS += -fsanitize=address
endif

.PHONY: all clean run gdb lldb test php js lines

-include Makefile.local

all: $(OBJS) main
	mkdir -p cc2
	./$< $(CONFIGS) --cpp-code > cc2/configuration.cc
	./$< $(CONFIGS) --cpp-header > cc2/configuration.h
	./$< $(CONFIGS) --cpp-json-code > cc2/configuration-json.cc
	./$< $(CONFIGS) --cpp-json-header > cc2/configuration-json.h

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

php: main test1.js $(CONFIGS)
	@OUTPUT=`./$< | php 2>&1;`; \
		if [[ -n "$$OUTPUT" ]]; then \
			./$< | cat -n; \
			echo -e "ERROR: [22;31m$$OUTPUT[0m"; \
			exit 1; \
		else \
			exit 0; \
		fi

	(./$< --php $(CONFIGS); cat ./test1.php) | php > /dev/null

js: main test1.js $(CONFIGS)
	@OUTPUT=`./$<  --js | node 2>&1;`; \
		if [[ -n "$$OUTPUT" ]]; then \
			./$< | cat -n; \
			echo -e "ERROR: [22;31m$$OUTPUT[0m"; \
			exit 1; \
		else \
			exit 0; \
		fi

	(./$< --js $(CONFIGS); cat ./test1.js) | node > /dev/null


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
	@rm -fv main;
