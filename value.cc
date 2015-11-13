/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include "value.h"

void Value::push(const Strings & s, const Type::TYPES t) {
  for (const auto & item : s) {
    properties.push_back({"", Value(t, item)});
  }
}

void Value::set(const bool c) {
  assert(type == Type::kBoolean);
  content = c ? "true" : "false";
}

void Value::set(const std::string & c) {
  assert(type == Type::kString);
  content = c;
}

void Value::set(std::string && c) {
  assert(type == Type::kString);
  content = std::move(c);
}

void Value::set(const char * const c) {
  assert(type == Type::kString);
  content = c;
}
