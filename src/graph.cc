/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <vector>

#include "graph.h"

struct EdgeSorter {
  const Graph & graph_;
  EdgeSorter(const Graph & g) : graph_(g) { }

  bool operator () (const Graph::edge_descriptor & a,
      const Graph::edge_descriptor & b) const {
    return graph_[a] < graph_[b];
  }
};

void contextSort(Graph & g) {
  typedef std::pair< Graph::edge_iterator, Graph::edge_iterator > Pair;
  Pair p = boost::edges(g);
  std::vector< Graph::edge_descriptor > edges(p.first, p.second);
  std::sort(std::begin(edges), std::end(edges), EdgeSorter(g));

  const size_t size = edges.size();

  for (size_t i = 0; i < size; ++i) {
    const Graph::edge_descriptor a = edges[i];
    for (size_t j = i + 1; j < size; ++j) {
      const Graph::edge_descriptor b = edges[j];
      if (g[a].contains(g[b])) {
        const Context r = g[b];
        edges.push_back(add_edge(target(a, g), target(b, g), r, g).first);
      }
    }
  }

  reverse(std::begin(edges), std::end(edges));

  size_t newSize = edges.size();
  size_t additionalSize = newSize - size;

  for (size_t i = 0; i < additionalSize; ++i) {
    const Graph::edge_descriptor a = edges[i];
    for (size_t j = i + 1; j < newSize; ++j) {
      const Graph::edge_descriptor b = edges[j];
      if (target(a, g) == target(b, g)) {
        remove_edge(b, g);
        edges.erase(std::begin(edges) + j);
        --newSize;
        if (j < additionalSize) {
          --additionalSize;
        }
      }
    }
  }
}
