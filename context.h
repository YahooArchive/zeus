/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>

struct Context {
  typedef std::vector< int > Dimensions;

  static const Context null;

  Dimensions dimensions;

  Context(void) { }
  Context(const Dimensions &);

  Context(const Context &) = default;
  Context(Context &&) = default;

  Context & operator = (const Context &) = default;
  Context & operator = (Context &&) = default;

  bool operator < (const Context &) const;

  bool operator == (const Context & c) const {
    return ! (*this < c || c < *this);
  }

  int operator & (const Context &) const;
  int operator [] (const int) const;

  bool contains(const Context &) const;
  int degree(void) const;
  int last(void) const;
};

typedef std::vector< Context > Contexts;

#endif
