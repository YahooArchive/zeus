/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef KEY_H
#define KEY_H

#include <map>
#include <string>

#include "graph.h"
#include "ir.h"
#include "structure.h"

struct Context;

struct Key {
  Graph graph;
  Graph::vertex_descriptor root_;
  Structure::ID type;
  ir::Kind kind;
  Structure::Alias alias;

  Key(void): graph(1), root_(*boost::vertices(graph).first),
    type(StructureTable::kUndefined),
    kind(ir::kNone) { }

  void add(const Context &, const Value &);
  void add(Context &&, const Value &);

  Value & root(void) {
    return graph[root_];
  }
};

struct KeyTable {
  struct Entry {
    int kid;
    Key key;
    int count;

    Entry(void) : kid(0) { }

    Entry(const Entry &) = default;
    Entry(Entry &&) = default;

    Entry & operator = (const Entry &) = default;
    Entry & operator = (Entry &&) = default;
  };

  typedef std::map< std::string, Entry > Entries;
  typedef std::map< int, Entry * > Index;

  int kid;
  Entries entries;
  Index index;

  KeyTable(void) : kid(0) { }

  bool insert(const std::string &);
  Entry & operator [] (const std::string &);
};

#endif //KEY_H
