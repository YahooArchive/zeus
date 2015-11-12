# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

CXX ?= g++
YAML_CPP_INCLUDE_PATH ?= ./yaml-cpp/include
CXXFLAGS += -g --std=c++11 -fPIC -Wall -I$(YAML_CPP_INCLUDE_PATH)
ENABLE_ASAN ?= false
GDB ?= gdb
CONFIGURATION ?= namespaces.yaml types.yaml dimensions.yaml master.yaml settings.yaml

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

cc: main
	mkdir -p cc2
	./$< $(CONFIGURATION) --cpp-code > cc2/configuration.cc
	./$< $(CONFIGURATION) --cpp-header > cc2/configuration.h
	./$< $(CONFIGURATION) --cpp-json-code > cc2/configuration-json.cc
	./$< $(CONFIGURATION) --cpp-json-header > cc2/configuration-json.h

run: main
	./$< $(CONFIGURATION);

gdb: main
	$(GDB) -ex run -ex quit --args ./$< $(CONFIGURATION);

lldb: main
	lldb -s ./lldb -- ./$< $(CONFIGURATION);

main: $(OBJS) yaml-cpp/libyaml-cpp.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

test: php js

php: main test1.js $(CONFIGURATION)
	@OUTPUT=`./$< | php 2>&1;`; \
		if [[ -n "$$OUTPUT" ]]; then \
			./$< | cat -n; \
			echo -e "ERROR: [22;31m$$OUTPUT[0m"; \
			exit 1; \
		else \
			exit 0; \
		fi

	(./$< --php $(CONFIGURATION); cat ./test1.php) | php > /dev/null

js: main test1.js $(CONFIGURATION)
	@OUTPUT=`./$<  --js | node 2>&1;`; \
		if [[ -n "$$OUTPUT" ]]; then \
			./$< | cat -n; \
			echo -e "ERROR: [22;31m$$OUTPUT[0m"; \
			exit 1; \
		else \
			exit 0; \
		fi

	(./$< --js $(CONFIGURATION); cat ./test1.js) | node > /dev/null

clean:
	rm -fv *.o main;

yaml-cpp/include/yaml-cpp/yaml.h yaml-cpp/CMakeLists.txt dep:
	git submodule update --init $<;

yaml-cpp/Makefile: yaml-cpp/CMakeLists.txt
	[ -r $@ ] || ( cd $(dir $<) && cmake .; );

yaml-cpp/libyaml-cpp.a: yaml-cpp/Makefile
	[ -r $@ ] || make -C $(dir $<);

$(OBJS) : %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@;

lines:
	wc -l *.cc *.h;
