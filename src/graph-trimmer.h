/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_TRIMMER_H
#define GRAPH_TRIMMER_H

#include "dimension.h"
#include "graph.h"

struct GraphTrimmer {
  typedef std::vector< int > Values;
  typedef std::vector< Values > Dimensions;

  Dimensions dimensions_;

  GraphTrimmer(Dimensions && d) :
    dimensions_(std::move(d)) { }

  GraphTrimmer(const Dimensions & d = Dimensions()) :
    dimensions_(d) { }

  void trim(Graph &) const;

  //TODO(dmorilha): horrible name
  void markSkip(dimensions::DimensionTable &) const;

  static GraphTrimmer::Dimensions Create(const dimensions::DimensionTable &,
      char * const * const, const int);
};

#endif //GRAPH_TRIMMER_H
