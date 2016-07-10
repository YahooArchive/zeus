/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>
#include <iostream>
#include "graph-type-extractor.h"
#include "structure.h"

ir::Kind GraphTypeExtractor::kind(const Value & v) const {
  switch (v.type) {
  case Type::kArray:
    return ir::kArray;

  case Type::kDynamic:
    return ir::kDynamic;

  default:
    break;
  }

  return ir::kNone;
}

void GraphTypeExtractor::extract(Key & k, StructureTable & s) const {
  Value & value = k.root();
  k.type = processValue(value, s);
  assert(value.alias.empty() || s[value.alias] != nullptr);
  k.alias = value.alias;
  k.kind = kind(value);
}

Structure::ID GraphTypeExtractor::processValue(Value & v,
    StructureTable & s) const {
  Structure structure;

  switch (v.type) {
  case Type::kUndefined:
    assert(false); //UNRECHEABLE
    break;
  case Type::kArray:
    {
      assert( ! v.properties.empty());
      auto & first = v.properties.front();
      const Structure::ID type = processValue(first.second, s);
      for (auto & item : v.properties) {
        assert(item.first == "");
        assert(item.second.type == first.second.type);
        //TODO(dmorilha): validate generated ID
      }
      return type;
    }
  case Type::kBoolean:
    return StructureTable::kBoolean;
  case Type::kDynamic:
    {
      assert( ! v.properties.empty());
      auto & first = v.properties.front();
      const Structure::ID type = processValue(first.second, s);
      for (auto & item : v.properties) {
        assert(item.first != "");
        assert(item.second.type == first.second.type);
        if (type >= StructureTable::kUserDefined) {
          item.second.content = first.second.content;
        }
        //TODO(dmorilha): validate generated ID
      }
      return type;
    }
  case Type::kFloat:
    return StructureTable::kFloat;
  case Type::kInteger:
    return StructureTable::kInteger;
  case Type::kObject:
    {
      Structure structure;
      assert( ! v.properties.empty());
      for (auto & item : v.properties) {
        assert(item.first != "");
        structure.addProperty(processValue(item.second, s),
            item.first, kind(item.second));
      }
      const StructureTable::Entry & entry = s.insert(structure);
      v.content = entry.getName();
      if ( ! v.alias.empty()) {
        if ( ! s.alias(entry.sid, v.alias)) {
          std::cerr << "Alias " << v.alias << " already exists for a different class" << std::endl;
          assert(false);
        }
      }
      return entry.sid;
    }
  case Type::kString:
    return StructureTable::kString;
  default:
    assert(false); //UNRECHEABLE
    break;
  }
  return StructureTable::kInvalid;
}
