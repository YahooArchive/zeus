/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef CPP_JSON_CODE_H
#define CPP_JSON_CODE_H

#include <string>

#include "generator.h"
#include "ir.h"

struct CPPJsonCodeGenerator : public Generator {
  typedef std::map< std::string, ir::Structure > Structures;

  void header(Printer &, const ir::Namespaces &, const bool k = false);
  void footer(Printer &, const ir::Namespaces &);

  void all(Printer &, const ir::Keys &);

  void key(Printer &, const ir::Key &, const Structures &);

  void generate(Printer &, const ir::Snapshot &);

  void content(Printer &, const Structures &, const std::string &,
      const ir::Kind &, const std::string &, const int);

  std::string type(const std::string &, const ir::Kind k = ir::kNone) const;

  bool nativeType(const std::string & s) const {
    return s == "boolean"
      || s == "float"
      || s == "integer"
      || s == "string";
  }
};

#endif //CPP_JSON_CODE_H
