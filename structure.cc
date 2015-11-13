/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>

#include <assert.h>

#include "structure.h"

Structure::Property::Property(const Structure::ID t, const std::string & i,
    const ir::Kind k):
  type(t),
  kind(k),
  identifier(i) { }

Structure::Property::Property(const Structure::ID t, std::string && i,
    const ir::Kind k):
  type(t),
  kind(k),
  identifier(std::move(i)) { }

bool Structure::Property::operator < (const Structure::Property & p) const {
  using namespace std;
  return lexicographical_compare(begin(identifier), end(identifier),
      begin(p.identifier), end(p.identifier));
}

//url enconded
std::string Structure::canonicalize(void) const {
  std::string result;
  for (const auto & item : properties) {
    result += "&"
      + item.identifier;
    if (item.kind == ir::kArray) {
      result += "[]";
    } else if (item.kind == ir::kDynamic) {
      result += "{}";
    }
    result += "=" + std::to_string(item.type);
  }
  return result;
}

StructureTable::Entry::Entry(const Structure & s, const Structure::ID id):
  structure(s),
  sid(id) { }

std::string StructureTable::Entry::getName(void) const {
  return std::string("Class_") + std::to_string(sid);
}

StructureTable::StructureTable(void):
  sid(kUserDefined) { }

StructureTable::Entry & StructureTable::operator [] (const Structure & s) {
  Entry & entry = entries[s.canonicalize()];

  if (entry.sid == kInvalid) {
    insert(s);
  }

  return entry;
}

StructureTable::Entry * StructureTable::operator [] (const Structure::ID i) {
  const auto iterator = index.find(i);
  return iterator != index.end() ? iterator->second : nullptr;
}

StructureTable::Entry & StructureTable::insert(const Structure & s) {
  Entry & entry = entries[s.canonicalize()];
  if (entry.sid < kUserDefined) {
    entry.sid = sid;
    entry.structure = s;
    index[sid] = &entry;
    ++sid;
  }
  return entry;
}

std::string StructureTable::getStructureName(const Structure::ID s) const {

  assert(s > kInvalid);

  if (s >= kUserDefined) {
    const auto iterator = index.find(s);
    assert(iterator != std::end(index));
    assert(iterator->second != nullptr);
    return iterator->second->getName();
  }

  return std::string();
}

std::string StructureTable::getTypeName(const Structure::ID s) const {
  assert(s > kInvalid);
  switch (s) {
  case StructureTable::kBoolean:
    return "boolean";

  case StructureTable::kFloat:
    return "float";

  case StructureTable::kInteger:
    return "integer";

  case StructureTable::kString:
    return "string";

  case StructureTable::kDynamic:
    return "";

  default:
    assert(s >= kUserDefined);
    const auto iterator = index.find(s);
    assert(iterator != std::end(index));
    assert(iterator->second != nullptr);
    return iterator->second->getName();
  }
}
