/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include "ir.h"

ir::Structure::Property::Property(const std::string & p, const std::string & t,
    const Kind k):
  property(p),
  type(t),
  kind(k) { }

ir::Structure::Property::Property(std::string && p, std::string && t,
    const Kind k):
  property(std::move(p)),
  type(std::move(t)),
  kind(k) { }

bool ir::Structure::Property::operator < (const ir::Structure::Property & p) const {
  using namespace std;
  return lexicographical_compare(begin(property), end(property),
      begin(p.property), end(p.property));
}

bool ir::Structure::operator < (const Structure & t) const {
  using namespace std;
  return lexicographical_compare(begin(identifier), end(identifier),
      begin(t.identifier), end(t.identifier));
}

ir::Key::Key(const ir::Key & k) :
  key(k.key),
  value(k.value),
  type(k.type),
  cache(k.cache),
  kind(k.kind),
  alias(k.alias) {

  if (static_cast< bool >(k.dimension)) {
    dimension.reset(new ir::Dimension(*k.dimension));
  }
}

bool ir::Key::operator < (const ir::Key & k) const {
  using namespace std;
  return lexicographical_compare(begin(key), end(key), begin(k.key), end(k.key));
}

ir::DimensionValue::DimensionValue(const ir::DimensionValue & d) :
  index(d.index),
  value(d.value) {

  if (static_cast< bool >(d.dimension)) {
    dimension.reset(new Dimension(*d.dimension));
  }
}

ir::Dimension::Dimension(const ir::Dimension & d) :
  dimension(d.dimension),
  values(d.values),
  skip(d.skip) {

  if (static_cast< bool >(d.next)) {
    next.reset(new Dimension(*d.next));
  }
}

bool ir::DimensionEnumeration::operator < (const ir::DimensionEnumeration & d) const {
  using namespace std;
  return lexicographical_compare(begin(dimension), end(dimension),
      begin(d.dimension), end(d.dimension));
}

