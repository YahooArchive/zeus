/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "graph.h"
#include "ir.h"

struct GraphBuilder {
  ir::Key build(const std::string &, Graph &, dimensions::DimensionTable &);
};
#endif //GRAPH_BUILDER_H
