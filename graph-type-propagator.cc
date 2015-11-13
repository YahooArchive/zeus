/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <iostream>

#include <boost/graph/depth_first_search.hpp>

#include "graph-type-propagator.h"

struct GraphTypePropagatorVisitor : boost::default_dfs_visitor {
  typedef std::vector< const Value * > Stack;
  Stack stack_;

  GraphTypePropagatorVisitor(void) { }

  void discover_vertex(Graph::vertex_descriptor v, const Graph & g) {
    const Value & value = g[v];
    if ( ! stack_.empty()) {
      processValue(value, stack_);
    }
    stack_.push_back(&value);
  }

  void finish_vertex(Graph::vertex_descriptor, const Graph &) {
    stack_.pop_back();
  }

  void processValue(const Value & v, const Stack & s, const bool m = true) {
    assert(s.front() != nullptr);
    const auto & first = *s.front();
    if (first.type == Type::kDynamic
        && v.type == Type::kObject) {
      const_cast< Value & >(v).type = Type::kDynamic;
    }
    assert(first.type == v.type);
    switch (v.type) {
      case Type::kUndefined:
        assert(false); //UNRECHEABLE
        break;

      case Type::kArray:
        {
          Stack stack;
          assert( ! first.properties.empty());
          const auto & first2 = first.properties.front();
          stack.push_back(&(first2.second));
          for (const auto & i : v.properties) {
            assert(i.first == "");
            if (i.second.type == Type::kObject) {
              assert(i.second.content.empty());
              const_cast< Value & >(i.second).content
                = first2.second.content;
            }
            processValue(i.second, stack, false);
          }
        }
        break;

      case Type::kDynamic:
        {
          assert( ! first.properties.empty());
          const auto & first2 = first.properties.front();
          //TODO(dmorilha): everything would be so much better if they were all sorted
          for (const auto & i : v.properties) {
            assert(i.first != "");
            assert(i.second.type == first2.second.type);
          }

          if (first2.second.type == Type::kObject) {
            for (const auto & i : v.properties) {
              Stack stack;

              if (m) {
                for (const auto & j : s) {
                  assert(j != nullptr);
                  for (const auto & k : j->properties) {
                    if (i.first == k.first) {
                      stack.push_back(&k.second);
                    }
                  }
                }
              }

              if (stack.empty()) {
                assert(i.second.content.empty());
                const_cast< Value & >(i.second).content
                  = first2.second.content;
                stack.push_back(&first2.second);
                processValue(i.second, stack, false);
              } else {
                processValue(i.second, stack, m);
              }
            }
          }
        }
        break;

      case Type::kObject:
        {
          assert( ! first.properties.empty());
          //TODO(dmorilha): everything would be so much better if they were all sorted
          for (const auto & i : v.properties) {
            assert(i.first != "");
            Stack stack;
            for (const auto & j : s) {
              assert(j != nullptr);
              for (const auto & k : j->properties) {
                if (i.first == k.first) {
                  stack.push_back(&k.second);
                  break;
                }
              }
            }
            if ( ! stack.empty()) {
              processValue(i.second, stack, m);
            }
          }
        }
        break;

      case Type::kString:
      case Type::kInteger:
      case Type::kFloat:
        if (static_cast< bool >(first.regex)) {
          if ( ! std::regex_match(v.content, *first.regex)) {
            std::cerr << "value \"" << v.content << "\" did not pass regular expression test" << std::endl;
            assert(false);
          }
        } else if (static_cast< bool >(first.set_)) {
          if (first.set_->find(v.content) == std::end(*first.set_)) {
            std::cerr << "value \"" << v.content << "\" did is not present into the set" << std::endl;
            assert(false);
          }
        }
        break;

      case Type::kBoolean:
        //nothing to be done.
        break;

      default:
        assert(false); //UNRECHEABLE
        break;
    }
  }
};

void GraphTypePropagator::propagate(Key & k) {
  GraphTypePropagatorVisitor visitor;
  boost::depth_first_search(k.graph, boost::visitor(visitor));
}
