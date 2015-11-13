/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <vector>

#include "graph-trimmer.h"

void GraphTrimmer::trim(Graph & g) const {
  const auto e = boost::edges(g);
  std::vector< Graph::edge_descriptor > edges(e.first, e.second);

  if (dimensions_.empty()) {
    return;
  }

  for (const auto & e : edges) {
    const auto & c = g[e];
    auto shouldRemove = false;

    const auto size = std::min(static_cast< size_t >(c.degree()),
        dimensions_.size());

    for (size_t i = 0; i < size; ++i) {
      const auto & dimension = dimensions_[i];
      if ( ! dimension.empty() && std::find(std::begin(dimension),
            std::end(dimension), c[i]) == std::end(dimension)) {
        shouldRemove = true;
        break;
      }
    }

    if (shouldRemove) {
      remove_edge(e, g);
    }
  }
}

void GraphTrimmer::markSkip(dimensions::DimensionTable & d) const {
  const auto size = dimensions_.size();
  for (size_t i = 0; i < size; ++i) {
    if (dimensions_[i].size() == 1) {
      //TODO(dmorilha) make it a method?
      d[i].skip = true;
    }
  }
}

GraphTrimmer::Dimensions GraphTrimmer::Create(const dimensions::DimensionTable & d,
    char * const * const argv, const int argc) {

  using namespace dimensions;

  GraphTrimmer::Dimensions result(d.size());

  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--set") == 0) {
      ++i;
      assert(i < argc);

      const auto length = strlen(argv[i]);

      char * const begin = argv[i];
      char * const end = begin + length;

      const char * const dimension = begin;

      char * value = std::find(begin, end, ':');
      *value = '\0';
      ++value;

      const DimensionTable::Entry * const p = d[dimension];
      if (p != nullptr) {
        const DimensionTable::Entry & entry = *p;
        while (value < end) {
          char * next = std::find(value, end, ',');
          *next= '\0';

          const ValueTable::Entry * const p2 = entry.values[value];

          if (p2 != nullptr) {
            assert(static_cast< size_t >(entry.did) < result.size());
            result[entry.did].push_back(p2->vid);
          } else {
            //invalid value
          }

          value = next + 1;
        }
      } else {
        //invalid dimension
      }
    }
  }
  return result;
}
