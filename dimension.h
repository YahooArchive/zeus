/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef DIMENSION_H
#define DIMENSION_H

#include <map>
#include <string>
#include <vector>

#include "context.h"
#include "ir.h"

namespace dimensions {

struct ValueTable {
  struct Entry {
    std::string value;
    int vid;
    int count;

    Entry(void): vid(0), count(0) { }

    explicit Entry(const std::string & v): value(v),
        vid(0), count(0) { }

    explicit Entry(std::string && v): value(std::move(v)),
        vid(0), count(0) { }

    Entry(const Entry &) = default;
    Entry(Entry &&) = default;

    Entry & operator = (const Entry &) = default;
    Entry & operator = (Entry &&) = default;
  };

  typedef std::map< std::string, Entry > Entries;
  typedef std::map< int, Entry * > Index;

  int vid;
  Entries entries;
  Index index;

  ValueTable(void) : vid(0) { }

  Entry * operator [] (const std::string &);
  const Entry * operator [] (const std::string &) const;

  Entry & insert(const std::string &);
};

struct DimensionTable {
  struct Entry {
    std::string dimension;
    int did;
    int count;
    ValueTable values;
    bool skip;

    Entry(void): did(0), count(0), skip(false) { };

    explicit Entry(const std::string & d): dimension(d),
        did(0), count(0), skip(false) { };

    explicit Entry(std::string && d): dimension(std::move(d)),
        did(0), count(0) { };

    Entry(const Entry &) = default;
    Entry(Entry &&) = default;

    Entry & operator = (const Entry &) = default;
    Entry & operator = (Entry &&) = default;
  };

  typedef std::vector< std::pair< std::string, std::string > > Input;

  typedef std::map< std::string, Entry > Entries;
  typedef std::map< int, Entry * > Index;

  int did;
  Entries entries;
  Index index;

  DimensionTable(void) : did(0) { }

  Entry & operator [] (const size_t);
  Entry * operator [] (const std::string &);
  const Entry * operator [] (const std::string &) const;

  Entry & insert(const std::string &);
  Context context(const Input &);
  Input lookup(const Context &);
  std::string & dimension(const size_t);
  ir::Dimensions enumerate(void) const;

  size_t size(void) const {
    assert(index.size() == entries.size());
    return entries.size();
  }
};

} // end of dimensions namespace

#endif
