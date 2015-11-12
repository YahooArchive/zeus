/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_TYPE_PROPAGATOR_H
#define GRAPH_TYPE_PROPAGATOR_H

#include "key.h"

struct GraphTypePropagator {
  void propagate(Key &);
};

#endif //GRAPH_TYPE_PROPAGATOR_H
