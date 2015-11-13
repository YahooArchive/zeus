/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>

#include <boost/range/adaptors.hpp>

#include "context.h"

const Context Context::null;

Context::Context(const Dimensions & d):
  dimensions(d) { }

bool Context::operator < (const Context & c) const {
  using namespace std;
  return lexicographical_compare(begin(dimensions), end(dimensions),
      begin(c.dimensions), end(c.dimensions));
}

int Context::operator & (const Context & c) const {
  const int size = dimensions.size();
  int depth = 0;

  for (int i = 0; i < size; ++i) {
    if (dimensions[i] == c.dimensions[i]) {
      depth++;

    } else {
      break;
    }
  }

  return depth;
}

int Context::operator [] (const int i) const {
  return dimensions[i];
}

bool Context::contains(const Context & c) const {
  const int size = dimensions.size();
  int i = 0;

  for (; i < size; ++i) {
    if (dimensions[i] != c.dimensions[i]) {
      break;
    }
  }

  for (; i < size; ++i) {
    if (dimensions[i] != 0) {
      return false;
    }
  }

  return true;
}

int Context::degree(void) const {
  using boost::adaptors::reverse;

  int degree = dimensions.size();

  for (const auto dimension : reverse(dimensions)) {
    if (dimension != 0) {
      break;
    }
    --degree;
  }

  return degree;
}

int Context::last() const {
  const int d = degree();
  assert(d >= 0);
  if (d == 0) {
    return 0;
  }
  return dimensions[d - 1];
}
