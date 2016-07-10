/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include "context.h"
#include "key.h"

/*
//TODO(dmorilha): review edge container choice
boost::adjacency_list< boost::vecS, boost::setS,
  boost::directedS, Value, Context > graph;
*/

void Key::add(const Context & c, const Value & v) {
  if (c == Context::null) {
    graph[root_] = v;
    return;
  }

  add_edge(root_, boost::add_vertex(v, graph), c, graph);
}

void Key::add(Context && c, const Value & v) {
  if (c == Context::null) {
    graph[root_] = v;
    return;
  }

  add_edge(root_, boost::add_vertex(v, graph),
      std::move(c), graph);
}

KeyTable::Entry & KeyTable::operator [] (const std::string & s) {
  Entry & entry = entries[s];

  if (entry.kid == 0) {
    const bool r = insert(s);
    assert(r);
  }

  return entry;
}

bool KeyTable::insert(const std::string & s) {
  //TODO(dmorilha): why not canonicalize on key.
  Entry & entry = entries[s];

  if (entry.kid != 0) {
    return false;
  }

  entry.kid = kid;
  index[kid] = &entry;

  ++kid;

  return true;
}
