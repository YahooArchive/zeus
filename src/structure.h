/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <map>
#include <set>
#include <string>

#include "ir.h"

struct Structure {
  typedef int ID;
  typedef std::string Alias;

  struct Property {
    ID type;
    ir::Kind kind;
    std::string identifier;

    Property(const ID, const std::string &, const ir::Kind k = ir::kNone);
    Property(const ID, std::string &&, const ir::Kind k = ir::kNone);

    Property(const Property &) = default;
    Property(Property &&) = default;

    Property & operator = (const Property &) = default;
    Property & operator = (Property &&) = default;

    bool operator < (const Property &) const;
  };

  typedef std::set< Property > Properties;
  typedef std::vector< Alias > Aliases;

  Properties properties;
  Aliases aliases;

  std::string canonicalize(void) const;

  template < class ... Args >
  bool addProperty(Args && ... arguments) {
    return properties.emplace(std::forward< Args >(arguments)...).second;
  }
};

struct StructureTable {
  enum Types {
    kInvalid,

    kUndefined,

    kBoolean,
    kFloat,
    kInteger,
    kString,
    kDynamic,

    kUserDefined = 100,
  };

  struct Entry {
    Structure structure;
    Structure::ID sid;

    Entry(void):
      sid(kUndefined) { };

    Entry(const Structure &, const Structure::ID);

    std::string getName(void) const;
  };

  typedef std::map< std::string, Entry > Entries;
  typedef std::map< Structure::ID, Entry * > Index;
  typedef std::map< Structure::Alias, Entry * > Aliases;

  Entries entries;
  Index index;
  Aliases aliases;

  Structure::ID sid;

  StructureTable(void);

  Entry & operator [] (const Structure &);
  Entry * operator [] (const Structure::ID);
  Entry * operator [] (const Structure::Alias &);

  Entry & insert(const Structure &);

  std::string getStructureName(const Structure::ID) const;
  std::string getTypeName(const Structure::ID) const;

  bool alias(const Structure::ID, const Structure::Alias &);
};

#endif
