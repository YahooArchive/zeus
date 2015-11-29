# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

CXX ?= g++
YAML_CPP_INCLUDE_PATH ?= ./yaml-cpp/include
CXXFLAGS += -g --std=c++11 -fPIC -Wall -I$(YAML_CPP_INCLUDE_PATH) -Wno-deprecated-declarations
ENABLE_ASAN ?= false
GDB ?= gdb
CONFIGS ?= $(wildcard conf/*.yaml)
OUTDIR := output
export BIN ?= zeus

ifeq ($(ENABLE_ASAN), true)
	CXXFLAGS += -fsanitize=address
endif

.PHONY: all clean run gdb lldb test php js lines

-include Makefile.local

all: $(OUTDIR)/configuration.cc $(OUTDIR)/configuration.h $(OUTDIR)/configuration-json.cc $(OUTDIR)/configuration-json.h $(OUTDIR)/configuration.js $(OUTDIR)/configuration.php

src/$(BIN):
src/$(BIN):
	$(MAKE) -C src $(BIN);

$(OUTDIR):
	@mkdir -vp $(OUTDIR)

$(OUTDIR)/configuration.cc: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --cpp-code > $(OUTDIR)/configuration.cc

$(OUTDIR)/configuration.h: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --cpp-header > $(OUTDIR)/configuration.h

$(OUTDIR)/configuration-json.cc: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --cpp-json-code > $(OUTDIR)/configuration-json.cc

$(OUTDIR)/configuration-json.h: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --cpp-json-header > $(OUTDIR)/configuration-json.h

$(OUTDIR)/configuration.js: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --js > $(OUTDIR)/configuration.js

$(OUTDIR)/configuration.php: $(BIN) $(OUTDIR)
	./$< $(CONFIGS) --php > $(OUTDIR)/configuration.php

run: $(BIN)
	./$< $(CONFIGS);

gdb: $(BIN)
	$(GDB) -ex run -ex quit --args ./$< $(CONFIGS);

lldb: $(BIN)
	lldb -s ./lldb -- ./$< $(CONFIGS);

$(BIN): src/$(BIN)
	@cp -fv $< $@;

test: php js

php: $(BIN) tests/test1.php $(CONFIGS)
	(./$< --php $(CONFIGS); cat tests/test1.php) | php > /dev/null

js: $(BIN) tests/test1.js $(CONFIGS)
	(./$< --js $(CONFIGS); cat tests/test1.js) | node > /dev/null

yaml-cpp/include/yaml-cpp/yaml.h yaml-cpp/CMakeLists.txt dep:
	git submodule update --init $<;

yaml-cpp/Makefile: yaml-cpp/CMakeLists.txt
	[ -r $@ ] || ( cd $(dir $<) && cmake .; );

yaml-cpp/libyaml-cpp.a: yaml-cpp/Makefile
	[ -r $@ ] || make -C $(dir $<);

clean:
	@$(MAKE) -C src clean;
	@rm -vfr $(BIN) output;
