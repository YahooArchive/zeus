/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef CPP_CODE_H
#define CPP_CODE_H

#include <string>

#include "generator.h"
#include "ir.h"

struct CPPCodeGenerator : public Generator {
  void header(Printer &, const ir::Namespaces &);
  void footer(Printer &, const ir::Namespaces &);
  void tables(Printer &, const ir::Dimensions &);
  void context(Printer &, const ir::Dimensions &);

  void key(Printer &, const ir::Key &, const ir::Dimensions &);
  void keyDimension(Printer &, const ir::Key &, const ir::Dimension &,
      const ir::Dimensions &, const int);

  void generate(Printer &, const ir::Snapshot &);

  void content(Printer &, const Type::TYPES,
      const std::string &);

  void value(Printer &, const Value &,
      const std::string &, const int);

  std::string type(const std::string &, const ir::Kind k = ir::kNone) const;

  void constructors(Printer &, const ir::Structure &);
};

#endif //CPP_CODE_H
