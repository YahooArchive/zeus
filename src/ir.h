/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef IR_HPP
#define IR_HPP

#include <assert.h>

#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "value.h"

/*
 * defines common interface between parser and different generators
 */

namespace ir {
  enum Kind {
    kNone,
    kArray,
    kDynamic,
  };

  struct DimensionEnumeration {
    typedef std::pair< std::string, unsigned int > Value;
    typedef std::vector< Value > Values;

    std::string dimension;
    Values values;

    bool operator < (const DimensionEnumeration &) const;
  };

  typedef std::map< std::string, DimensionEnumeration > Dimensions;

  struct Dimension;

  typedef std::unique_ptr< Dimension > DimensionPointer;

  struct DimensionValue {
    unsigned int index;
    DimensionPointer dimension;
    Value value;

    template < class T1 >
    DimensionValue(const int, T1 && v, DimensionPointer && d = nullptr);

    DimensionValue(const int i):
      index(i),
      dimension(nullptr) { }

    DimensionValue(const DimensionValue &);
  };

  typedef std::vector< DimensionValue > DimensionValues;

  template < class T1 >
  DimensionValue::DimensionValue(const int i, T1 && v, DimensionPointer && d):
    index(i),
    dimension(std::move(d)),
    value(std::forward< T1 >(v)) { }

  struct Dimension {
    std::string dimension;
    DimensionValues values;
    DimensionPointer next;
    bool skip;

    Dimension(std::string && d, const bool s = false) :
      dimension(std::move(d)),
      next(nullptr),
      skip(s) { }

    Dimension(const std::string & d, const bool s = false) :
      dimension(d),
      next(nullptr),
      skip(s) { }

    template < class T1, class T2 >
    Dimension(T1 && d, T2 && v, DimensionPointer && n = nullptr) :
      dimension(std::forward< T1 >(d)),
      values(std::forward< T2 >(v)),
      next(std::move(n)),
      skip(false) { }

    Dimension(const Dimension &);
  };

  struct Key {
    std::string key;
    Value value;
    std::string type;
    DimensionPointer dimension;
    bool cache;
    Kind kind;
    std::string alias;

    template < class T1, class T2, class T3 >
    Key(T1 && k, T2 && v, T3 && t, DimensionPointer && d) :
      key(std::forward< T1 >(k)),
      value(std::forward< T2 >(v)),
      type(std::forward< T3 >(t)),
      dimension(std::move(d)),
      cache(true),
      kind(kNone) { }

    explicit Key(std::string && k) :
      key(std::move(k)),
      cache(true),
      kind(kNone) { }

    explicit Key(const std::string & k) :
      key(k),
      cache(true),
      kind(kNone) { }

    Key(const Key &);

    Key & operator = (Key &&) = default;
    bool operator < (const Key &) const;

    bool operator == (const char * const k) const {
      return key == k;
    }
  };

  typedef std::vector< Key > Keys;

  struct Structure {
    struct Property {
      std::string property;
      std::string type;
      Kind kind;

      struct {
        std::string declaration;
      } comments;

      Property(const std::string &, const std::string & t = std::string(),
          const Kind k = kNone);

      Property(std::string &&, std::string && t = std::string(),
          const Kind k = kNone);

      Property(const Property &) = default;
      Property(Property &&) = default;

      Property & operator = (const Property &) = default;
      Property & operator = (Property &&) = default;
      bool operator < (const Property &) const;
    };

    typedef std::vector< Property > Properties;
    typedef std::vector< std::string > Aliases;

    std::string identifier;
    Properties properties;
    Aliases aliases;

    template < class T1, class T2, class T3 >
    Structure(T1 &&, T2 &&, T3 &&);

    bool operator < (const Structure &) const;
  };

  typedef std::vector< Structure > Structures;

  template < class T1, class T2, class T3 >
  Structure::Structure(T1 && t, T2 && p, T3 && a) :
    identifier(std::forward< T1 >(t)),
    properties(std::forward< T2 >(p)),
    aliases(std::forward< T3 >(a)) { }

  typedef std::vector< std::string > Namespaces;

  struct Snapshot {
    Dimensions dimensions;
    Keys keys;
    Structures structures;
    Namespaces namespaces;
  };
}

#endif
