# Copyright (c) 2015, Yahoo Inc. All rights reserved.
# Copyrights licensed under the New BSD License.
# See the accompanying LICENSE file for terms.

LIB_NAME = ats-zeus
PLUGIN_TAG ?= $(LIB_NAME)

CXXFLAGS += -DPLUGIN_TAG=\"$(PLUGIN_TAG)\"
CXXFLAGS += -fPIC

SHLIB_VERSION = 1
PACKAGE_VERSION = $(SHLIB_VERSION).0

O += context.o library.o

.PRECIOUS: %.o

-include Makefile.local

all: $(LIB_NAME).so

%.so.debug : CXXFLAGS += -O0 -g3 --coverage
%.so.debug : %.o.debug $(addsuffix .debug,$O)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -shared -o $@ $^;

%.so : CXXFLAGS += -O2 -DNDEBUG
%.so : %.o $O
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -shared -o $@ $^;

%.o : %.cc
	$(CXX) -c  $(CXXFLAGS) -o $@ $<;

%.o.debug : %.cc
	$(CXX) -c  $(CXXFLAGS) -o $@ $<;

clean:
	@rm -fv *.so *o.debug *.gcno;
