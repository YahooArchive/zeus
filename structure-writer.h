/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef STRUCTURE_WRITER_H
#define STRUCTURE_WRITER_H

#include "structure.h"
#include "ir.h"

struct StructureWriter {
  void operator () (const StructureTable &, ir::Structures &) const;
};

#endif
