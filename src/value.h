/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <string>
#include <vector>

#include "common.h"

struct Type {
  enum TYPES {
    kUndefined,
    kArray,
    kBoolean,
    kDynamic,
    kFloat,
    kInteger,
    kObject,
    kString,
    kUnknown,
  };

  static std::string ToString(const TYPES t) {
    switch (t) {
    case kUndefined:
      return "undefined";
    case kArray:
      return "array";
    case kBoolean:
      return "boolean";
    case kDynamic:
      return "dynamic";
    case kFloat:
      return "float";
    case kInteger:
      return "integer";
    case kObject:
      return "object";
    case kString:
      return "string";
    case kUnknown:
      return "unknown";
    default:
      assert(false); //UNRECHEABLE
    }
  }
};

struct Value {
  typedef std::pair< std::string, Value > Property;
  typedef std::vector< Property > Properties;

  typedef std::vector< bool > Booleans;
  typedef std::vector< double > Floats;
  typedef std::vector< int > Integers;
  typedef std::vector< std::string > Strings;

  Type::TYPES type;
  bool reset;
  std::string content;
  Properties properties;
  Regex regex;
  Set set_;
  std::string alias;
  bool ignore;

  Value(void): type(Type::kUndefined), reset(false), ignore(false) { }

  Value(const Type::TYPES t, const std::string & c, const bool r = false):
    type(t), reset(r), content(c), ignore(false) { }

  Value(const Type::TYPES t, std::string && c, const bool r = false):
    type(t), reset(r), content(std::move(c)), ignore(false) { }

  Value(const Type::TYPES t, const Strings & c, const bool r = false):
    type(Type::kArray), reset(r), ignore(false) { push(c, t); }

  Value(const Type::TYPES t, Strings && c, const bool r = false):
    type(Type::kArray), reset(r), ignore(false) { push(c, t); }

  Value(const std::string & c, const bool r = false):
    type(Type::kString), reset(r), content(c), ignore(false) { }

  Value(const char * const c, const bool r = false):
    type(Type::kString), reset(r), content(c), ignore(false) { }

  Value(std::string && c, const bool r = false):
    type(Type::kString), reset(r), content(std::move(c)), ignore(false) { }

  Value(const Booleans & c, const bool r = false):
    type(Type::kArray), reset(r), ignore(false) {
    properties.reserve(c.size());
    for (const auto item : c) {
      properties.push_back({"",
          Value(Type::kBoolean, item ? "true" : "false")});
    }
  }

  Value(const bool c):
    type(Type::kBoolean), reset(false), content(c ? "true" : "false"),
    ignore(false) { }

  Value(const Floats &, const bool r = false);

  Value(const Integers &, const bool r = false);

  Value(const float c):
    type(Type::kFloat), reset(false), content(std::to_string(c)),
    ignore(false) { }

  Value(const double c):
    type(Type::kFloat), reset(false), content(std::to_string(c)),
    ignore(false) { }

  template < class T >
  Value(const T c):
    type(Type::kInteger), reset(false), content(std::to_string(c)),
    ignore(false) { }

  Value(const Value::Properties & p) :
    type(Type::kUndefined), reset(false), properties(p), ignore(false) { }

  Value(Properties && p) :
    type(Type::kUndefined), reset(false), properties(std::move(p)),
    ignore(false) { }

  Value(const Value &) = default;
  Value(Value &&) = default;

  Value & operator = (const Value &) = default;
  Value & operator = (Value &&) = default;

  void push(const Strings &, Type::TYPES t = Type::kString);

  template < class T >
  Property & push(T && c) {
    assert(type == Type::kArray);
    properties.push_back({"", Value(std::forward< T >(c))});
    return properties.back();
  }

  template < class T >
  Property & push(Type::TYPES t, T && c) {
    assert(type == Type::kArray);
    properties.push_back({"", Value(t, std::forward< T >(c))});
    return properties.back();
  }

  Property & push(void) {
    assert(type == Type::kArray
        || type == Type::kDynamic
        || type == Type::kObject);
    properties.push_back({});
    return properties.back();
  }

  void set(const bool);
  void set(const std::string &);
  void set(std::string &&);
  void set(const char * const);

  template < class T >
  void set(const T & c) {
    assert(type == Type::kFloat || type == Type::kInteger);
    if ( ! content.empty()) {
      content.clear();
    }
    content.push_back(std::to_string(c));
  }
};

#endif
