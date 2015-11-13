/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>

#include "dimension.h"

namespace dimensions {

ValueTable::Entry & ValueTable::insert(const std::string & s) {
  const auto result = entries.emplace(s, Entry(s));
  auto & entry = std::get< 0 >(result)->second;

  if (std::get< 1 >(result)) {
    entry.vid = vid;
    index[vid] = &entry;
    ++vid;
  }

  ++entry.count;
  return entry;
}

DimensionTable::Entry & DimensionTable::insert(const std::string & s) {
  const auto result = entries.emplace(s, Entry(s));
  auto & entry = std::get< 0 >(result)->second;

  if (std::get< 1 >(result)) {
    entry.did = did;
    index[did] = &entry;
    ++did;

    entry.values.insert("NONE");
  }

  ++entry.count;
  return entry;
}

Context DimensionTable::context(const DimensionTable::Input & i) {
  Context::Dimensions dimensions(entries.size(), 0);

  for (const auto & item : i) {
    auto & entry = insert(item.first);
    dimensions[entry.did] = entry.values.insert(item.second).vid;
  }

  return Context(std::move(dimensions));
}

//TODO(dmorilha): the index subscriptor alters the object
//preventing this function to be const.
DimensionTable::Input DimensionTable::lookup(const Context & c) {
  DimensionTable::Input input;
  for (size_t i = 0; i < c.dimensions.size(); ++i) {
    input.push_back(std::make_pair(
          index[i]->dimension,
          index[i]->values.index[c.dimensions[i]]->value));
  }
  return input;
}

DimensionTable::Entry & DimensionTable::operator [] (const size_t i) {
  assert(i < index.size());
  const auto result = index[i];
  assert(result != nullptr);
  return *result;
}

DimensionTable::Entry * DimensionTable::operator [] (const std::string & n) {
  DimensionTable::Entry * result = nullptr;
  const auto iterator = entries.find(n);
  if (iterator != entries.end()) {
    result = &iterator->second;
  }
  return result;
}

const DimensionTable::Entry * DimensionTable::operator [] (const std::string & n) const {
  const DimensionTable::Entry * result = nullptr;
  const auto iterator = entries.find(n);
  if (iterator != entries.end()) {
    result = &iterator->second;
  }
  return result;
}

std::string & DimensionTable::dimension(const size_t i) {
  return operator [](i).dimension;
}

ir::Dimensions DimensionTable::enumerate(void) const {
  ir::Dimensions dimensions;
  for (const auto & entry : entries) {
    ir::DimensionEnumeration::Values values;
    for (const auto & item : entry.second.values.index) {
      assert(item.second != NULL);
      values.push_back({item.second->value, item.second->vid});
    }
    dimensions.emplace(entry.first, ir::DimensionEnumeration(
          {entry.first, values}));
  }
  return dimensions;
}

const ValueTable::Entry * ValueTable::operator [] (const std::string & n) const {
  const ValueTable::Entry * result = nullptr;
  const auto iterator = entries.find(n);
  if (iterator != entries.end()) {
    result = &iterator->second;
  }
  return result;
}

} //end of dimensions namespace
