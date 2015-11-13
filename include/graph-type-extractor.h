/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef GRAPH_TYPE_EXTRACTOR_H
#define GRAPH_TYPE_EXTRACTOR_H

#include "key.h"
#include "structure.h"
#include "value.h"

struct GraphTypeExtractor {
  void extract(Key &, StructureTable &) const;
  Structure::ID processValue(Value &, StructureTable &) const;
  ir::Kind kind(const Value &) const;
};
#endif //GRAPH_TYPE_EXTRACTOR_H
