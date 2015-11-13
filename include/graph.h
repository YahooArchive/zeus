/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <boost/graph/adjacency_list.hpp>

#include "context.h"
#include "dimension.h"
#include "value.h"

typedef boost::adjacency_list< boost::vecS, boost::vecS,
  boost::directedS, Value, Context > Graph;

void contextSort(Graph &);

#endif //GRAPH_H
