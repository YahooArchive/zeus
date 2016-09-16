/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef PYTHON_H
#define PYTHON_H

#include "generator.h"
#include "ir.h"

struct PythonGenerator : public Generator {
  void header(Printer &, const ir::Namespaces &);

  void structure(Printer &, const ir::Structure &, const ir::Namespaces &);

  void dimension(Printer &, const ir::DimensionEnumeration &);

  void key(Printer &, const ir::Key &, const ir::Dimensions &);
  void keyDimension(Printer &, const ir::Key &, const ir::Dimension &,
      const ir::Dimensions &, const int);

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

#endif //PYTHON_H
