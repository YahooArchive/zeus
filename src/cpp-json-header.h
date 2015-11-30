/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef CPP_JSON_HEADER_H
#define CPP_JSON_HEADER_H

#include <string>

#include "generator.h"
#include "ir.h"

struct CPPJsonHeaderGenerator : public Generator {
  void header(Printer &, const ir::Namespaces &);
  void footer(Printer &, const ir::Namespaces &);

  void structure(Printer &, const ir::Structure &);

  void dimension(Printer &, const ir::DimensionEnumeration &);
  void dimensionClass(Printer &);

  void key(Printer &, const ir::Key &);

  void contextClass(Printer &, const ir::Snapshot &);

  void configurationClass(Printer &, const ir::Snapshot &);

  void generate(Printer &, const ir::Snapshot &);

  std::string type(const std::string &, const bool a = false) const;
};

#endif //CPP_JSON_HEADER_H
