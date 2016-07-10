/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <boost/graph/depth_first_search.hpp>

#include "context.h"
#include "graph-printer.h"

struct PrinterVisitor : boost::default_dfs_visitor {
  dimensions::DimensionTable & dimensionTable_;
  std::ostream & output_;
  std::string prefix_;

  PrinterVisitor(dimensions::DimensionTable & d, std::ostream & o) :
    dimensionTable_(d), output_(o) { }

  void discover_vertex(Graph::vertex_descriptor a, const Graph & g) const {
    const Value & v = g[a];
    GraphPrinter::printValue(v, output_);
    output_ << "\n";
  }

  void finish_vertex(Graph::vertex_descriptor a, const Graph & g) {
    prefix_.erase(0, 3);
  }

  void start_vertex(Graph::vertex_descriptor, const Graph &) const {
    output_ << ".";
  }

  void tree_edge(Graph::edge_descriptor a, const Graph & g) {
    const Context & c = g[a];
    output_ << prefix_ << "├─ ";
    GraphPrinter::printContext(c, dimensionTable_, output_);
    prefix_ += "|  ";
  }
};

void GraphPrinter::print(const Graph & g,
    dimensions::DimensionTable & d) const {
  PrinterVisitor visitor(d, output_);
  boost::depth_first_search(g, boost::visitor(visitor));
}

void GraphPrinter::printContext(const Context & c,
    dimensions::DimensionTable & d, std::ostream & o) {
  typedef dimensions::DimensionTable::Input Input;
  Input input = d.lookup(c);

  if ( ! input.empty()) {
    o << "(" << input[0].first << ": " << input[0].second;
    for (size_t i = 1; i < input.size(); ++i) {
      o << ", " << input[i].first << ": " << input[i].second;
    }
    o << ") ";
  }
}

void GraphPrinter::printValue(const Value & v, std::ostream & o) {
  if (v.type == Type::kArray) {
    const size_t size = v.properties.size();
    if (size > 0) {
      o << "(sequence) ";
      o << v.properties[0].second.content;
      for (size_t i = 1; i < size; ++i) {
        o << ", " << v.properties[i].second.content;
      }
    }
  } else {
    o << v.content;
  }
}
