/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef JS_H
#define JS_H

#include "generator.h"
#include "ir.h"

struct JSGenerator : public Generator {
  void header(Printer &);
  void footer(Printer &);

  void structure(Printer &, const ir::Structure &);

  void dimension(Printer &, const ir::DimensionEnumeration &);
  void dimensionClass(Printer &);

  void key(Printer &, const ir::Key &);
  void keyDimension(Printer &, const ir::Key &,
      const ir::Dimension &, const int);

  void configurationClass(Printer &, const ir::Snapshot &);

  void generate(Printer &, const ir::Snapshot &);

  void content(Printer &, const Type::TYPES,
      const std::string &);

  void value(Printer &, const Value &,
      const std::string &, const int);

  bool nativeType(const std::string & s) const {
    return s == "boolean"
      || s == "float"
      || s == "integer"
      || s == "string";
  }
};

#endif //JS_H
