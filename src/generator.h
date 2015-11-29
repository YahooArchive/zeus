/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <memory>
#include <ostream>
#include <string>

#include "ir.h"

struct tab {
  const int index;
  tab(const int index) :
    index(index) { }
};

struct Printer {
  std::ostream & o;

  Printer(std::ostream & o) :
    o(o) { }

  template < class T >
  Printer & operator << (const T & t) {
    o << t;
    return *this;
  }

  Printer & operator << (const tab &);
};

struct Generator {
  typedef std::unique_ptr< Generator > Pointer;

  virtual ~Generator() { }
  Generator(void) { }

  virtual void generate(Printer &, const ir::Snapshot &) = 0;

  std::string constantify(const std::string &);
  std::string identifier(const std::string &);
};

#endif
