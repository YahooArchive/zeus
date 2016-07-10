/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include "structure-writer.h"

void StructureWriter::operator () (const StructureTable & t, ir::Structures & s) const {
  for (const auto & item : t.entries) {
    ir::Structure::Properties properties;
    const auto & entry = item.second;

    for (const auto & item : entry.structure.properties) {
      ir::Structure::Property property(item.identifier,
          t.getStructureName(item.type), item.kind);

      if (item.type < StructureTable::kUserDefined) {
        property.type = t.getTypeName(item.type);
      }

      properties.emplace_back(property);
    }

    s.emplace_back(entry.getName(), properties, entry.structure.aliases);
  }
}
