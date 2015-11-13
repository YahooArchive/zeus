/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <memory>

#include "generator.h"

Printer & Printer::operator << (const tab & t) {
  for (int i = 0; i < t.index; ++i) {
    *this << "  ";
  }

  return *this;
}

std::string Generator::constantify(const std::string & s) {
  using namespace std;
  string r = identifier(s);
  transform(begin(r), end(r),
      begin(r), ::toupper);
  return r;
}

std::string Generator::identifier(const std::string & s) {
  using namespace std;
  string r;
  transform(begin(s), end(s),
      back_inserter(r), [](char a) -> char {
        return isalnum(a) || a == '\0' ? a : '_';
      }
  );
  return r;
}
