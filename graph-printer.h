/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_PRINTER_H
#define GRAPH_PRINTER_H

#include <ostream>

#include "dimension.h"
#include "graph.h"

struct GraphPrinter {
  std::ostream & output_;
  GraphPrinter(std::ostream & o) : output_(o) { }

  static void printContext(const Context &,
      dimensions::DimensionTable &, std::ostream &);

  static void printValue(const Value & v, std::ostream & o);

  void print(const Graph &, dimensions::DimensionTable &) const;
};

#endif //GRAPH_PRINTER_H
