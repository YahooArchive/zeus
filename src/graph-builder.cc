/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <boost/graph/depth_first_search.hpp>

#include "graph-builder.h"

struct GraphBuilderVisitor : boost::default_dfs_visitor {
  typedef std::vector< ir::DimensionPointer * > Stack;

  ir::Key & key_;
  dimensions::DimensionTable & table_;
  int degree_;
  const Context * last_;
  ir::DimensionPointer * current_;
  Stack stack_;

  GraphBuilderVisitor(ir::Key & k, dimensions::DimensionTable & d) :
    key_(k), table_(d), degree_(0),
    last_(nullptr), current_(nullptr) { }

  void tree_edge(Graph::edge_descriptor a, const Graph & g) {
    const Context & c = g[a];

    if (last_ != nullptr) {
      const int d = *last_ & c;
      for (; degree_ > d; --degree_) {
        current_ = stack_.back();
        stack_.pop_back();
      }
    }

    const int d = c.degree();

    for (; degree_ < d; ++degree_) {
      if ( ! static_cast< bool >(*current_)) {
        const auto & dimension = table_[degree_];
        (*current_).reset(new ir::Dimension(dimension.dimension,
              dimension.skip));
      }

      if (c[degree_] > 0) {
        (*current_)->values.push_back(ir::DimensionValue(c[degree_]));
        stack_.push_back(current_);
        current_ = &(*current_)->values.back().dimension;
      } else {
        stack_.push_back(current_);
        current_ = &(*current_)->next;
      }
    }

    last_ = &c;
  }

  void discover_vertex(Graph::vertex_descriptor v, const Graph & g) {
    if ( ! stack_.empty()) {
      (*stack_.back())->values.back().value = g[v];
    }
  }

  void finish_vertex(Graph::vertex_descriptor, const Graph &) {
    if ( ! stack_.empty()) {
      --degree_;
      current_ = stack_.back();
      stack_.pop_back();
    }
  }

  void start_vertex(Graph::vertex_descriptor v, const Graph & g) {
    key_.value = g[v];
    current_ = &key_.dimension;
  }
};

ir::Key GraphBuilder::build(const std::string & s,
    Graph & g, dimensions::DimensionTable & d) {
  ir::Key key(s);
  GraphBuilderVisitor visitor(key, d);
  boost::depth_first_search(g, boost::visitor(visitor));
  return key;
}
