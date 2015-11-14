/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>

#include <assert.h>

#include "js.h"

void JSGenerator::structure (Printer & p, const ir::Structure & structure) {
  const std::string id = identifier(structure.identifier);

  p << tab(1) << "function " << id << "() {" << "\n";

  {
    ir::Structure::Properties properties = structure.properties;
    std::sort(std::begin(properties), std::end(properties));

    for (const auto & property : properties) {
      p << tab(2) << "this." << identifier(property.property) << " = ";

      if (property.kind == ir::kArray) {
        p << "[]";

      } else if (property.kind == ir::kDynamic) {
        p << "{}";

      } else if (property.type == "boolean") {
        p << "false";

      } else if (property.type == "float"
          || property.type == "integer") {
        p << "0";

      } else if (property.type == "string") {
        p << "\"\"";

      } else {
        p << "new " << identifier(property.type) << "()";
      }

      p << ";";

      if ( ! property.comments.declaration.empty()) {
        p << " // " << property.comments.declaration;
      } else if ( ! property.type.empty()) {
        p << " // type: " << property.type;
        if (property.kind == ir::kArray) {
          p << " (array)";
        } else if (property.kind == ir::kDynamic) {
          p << " (dynamic)";
        }
      }

      p << "\n";
    }

    p << tab(1) << "}" << "\n"
      << "\n"
      << tab(1) << id
      << ".prototype = ConfigurationStructure;" << "\n";

    for (const auto & item : structure.aliases) {
      p << tab(1) << "var " << item << " = " << id << ";" << "\n";
    }

    p << "\n";
  }
}

void JSGenerator::content(Printer & p, const Type::TYPES t,
    const std::string & c) {

  switch (t) {
  case Type::kBoolean: {
    using namespace std;

    string content;

    transform(begin(c), end(c),
        back_inserter(content), ::tolower);

    assert(content == "true" || content == "false");

    p << content;
    break;
  }

  case Type::kFloat:
  case Type::kInteger:
    p << c;
    break;

  case Type::kObject:
    p << "new " << c << "()";
    break;

  case Type::kString:
    p << "\"" << c << "\"";
    break;

  case Type::kUndefined:
    //std::cerr << "type is undefined" << std::endl;
    break;

  default:
    //std::cerr << "ignoring unknown type" << std::endl;
    break;
  }
}

void JSGenerator::value(Printer & p, const Value & value,
    const std::string & prefix, const int t) {

  if ( ! value.properties.empty()) {
    if (value.type == Type::kArray) {
      for (const auto & item : value.properties) {
        p << tab(t) << prefix << ".push(";
        content(p, item.second.type, item.second.content);
        p << ");" << "\n";
      }
    } else if (value.type == Type::kObject) {
      for (const auto & item : value.properties) {
        this->value(p, item.second, prefix + "." + item.first, t);
      }
    } else if (value.type == Type::kDynamic) {
      for (const auto & item : value.properties) {
        const std::string dynamicPrefix = prefix + "['" + item.first + "']";
        if (item.second.type == Type::kArray) {
          p << tab(t) << dynamicPrefix << " = [];" << "\n";
        } else if (item.second.type == Type::kDynamic) {
          p << tab(t) << dynamicPrefix << " = {};" << "\n";
        } else if (item.second.type == Type::kObject
            && ! item.second.content.empty()) {
          p << tab(t) << dynamicPrefix << " = new "
            << item.second.content << "();" << "\n";
        }
        this->value(p, item.second, dynamicPrefix, t);
      }
    } else {
      assert(false);
    }
  } else if (value.type != Type::kUndefined) {
    assert( ! value.content.empty() || value.type == Type::kString);
    p << tab(t) << prefix << " = ";
    content(p, value.type, value.content);
    p << ";" << "\n";
  }
}

void JSGenerator::keyDimension(Printer & p, const ir::Key & key,
    const ir::Dimension & dimension, const int t) {

  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "switch (this." << identifier(dimension.dimension) << ") {" << "\n";
  } else {
    assert(dimension.values.size() == 1 ||
        (dimension.values.empty() && dimension.next != nullptr));
    p << tab(t) << "//skipping dimension " << dimension.dimension << "\n";
  }

  for (const auto & item : dimension.values) {
    assert(item.index > 0);

    if ( ! dimension.skip) {
      p << tab(t) << "case " << item.index << ":" << "\n";
    }

    value(p, item.value, "value", t + (dimension.skip ? 0 : 1));

    if (static_cast< bool >(item.dimension)) {
      keyDimension(p, key, *item.dimension, t + (dimension.skip ? 0 : 1));
    }

    if ( ! dimension.skip) {
      p << tab(t + 1) << "break;" << "\n";
    }
  }

  if (static_cast< bool >(dimension.next)) {
    if (dimension.skip || dimension.values.empty()) {
      keyDimension(p, key, *dimension.next, t);
    } else {
      p << tab(t) << "default:" << "\n";
      keyDimension(p, key, *dimension.next, t + 1);
      p << tab(t + 1) << "break;" << "\n";
    }
  }

  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "}" << "\n";
  }
}

void JSGenerator::key(Printer & p, const ir::Key & key) {
  p << tab(2) << key.key << ": function () {" << "\n"
    << tab(3) << "var value;" << "\n";

  if (key.cache) {
    p << tab(3) << "if (this._cache.hasOwnProperty(\""
      << key.key << "\")) {" << "\n"
      << tab(4) << "return this._cache[\""
      << key.key << "\"];" << "\n"
      << tab(3) << "}" << "\n";
  }

  if (key.value.type == Type::kArray) {
    p << tab(3) << "value = [];" << "\n";
  } else if (key.value.type == Type::kDynamic) {
    p << tab(3) << "value = {};" << "\n";
  } else if ( ! key.alias.empty()) {
    p << tab(3) << "value = new " << key.alias << "();" << "\n";
  } else if ( ! nativeType(key.type)) {
    p << tab(3) << "value = new " << key.type << "();" << "\n";
  }

  value(p, key.value, "value", 3);

  if (static_cast< bool >(key.dimension)) {
    keyDimension(p, key, *key.dimension, 3);
  }

  p << tab(3) << "return ";

  if (key.cache) {
    p << "this._cache[\"" << key.key << "\"] = ";
  }

  p << "value;" << "\n"
    << tab(2) << "}," << "\n"
    << "\n";
}

void JSGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {

  p << tab(1) << "function Configuration(d) {" << "\n"
    << tab(2) << "this._cache = {};" << "\n";

  {
    const ir::Dimensions & dimensions = snapshot.dimensions;

    for (const auto & dimension : dimensions) {
      p << tab(2) << "this." << identifier(dimension.first) << " = "
        << constantify(dimension.first) << "[d[\"" << dimension.first << "\"]] || 0;" << "\n";
    }
  }

  p << tab(1) << "}" << "\n"
    << "\n";

  {
    ir::Keys keys = snapshot.keys;
    std::sort(std::begin(keys), std::end(keys));

    p << tab(1) << "//print each configuration key" << "\n";
    p << tab(1) << "Configuration.prototype = {" << "\n";

    for (const auto & key : keys) {
      this->key(p, key);
    }
  }

  p << tab(1) <<  "};" << "\n";
}

void JSGenerator::dimension(Printer & p, const ir::DimensionEnumeration & dimension) {

  const std::string className = constantify(dimension.dimension);

  ir::DimensionEnumeration::Values values = dimension.values;
  std::sort(std::begin(values), std::end(values));

  p << tab(1) << "//print enumeration" << "\n"
    << tab(1) << "var " << className << " = {" << "\n";

  for (const auto & value : values) {
    const auto constant = constantify(value.first);

    if (constant != value.first) {
      p << tab(2) << constantify(value.first) << ": " << value.second << "," << "\n";
    }

    p << tab(2) << "\"" << value.first << "\": " << value.second << "," << "\n";
  }

  p << tab(1) << "};" << "\n"
    << "\n";
}

void JSGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {

  header(p);

  {
    ir::Structures structures = snapshot.structures;

    if ( ! structures.empty()) {
      //php allows declarings classes in any sequence.
      std::sort(std::begin(structures), std::end(structures));

      p << tab(1) << "var ConfigurationStructure = { };" << "\n"
        << "\n";

      for (const auto & item : structures) {
        structure(p, item);
      }
    }
  }

  for (const auto & dimension : snapshot.dimensions) {
    this->dimension(p, dimension.second);
  }

  configurationClass(p, snapshot);

  footer(p);
}

void JSGenerator::header(Printer & p) {
  p << "var Configuration = (function() {" << "\n"
    << tab(1) << "\"use strict\";" << "\n";
}

void JSGenerator::footer(Printer & p) {
  p << tab(1) << "return Configuration;" << "\n"
    << "}());" << "\n"
    << "\n"
    << "if (exports) {" << "\n"
    << tab(1) << "exports.Configuration = Configuration;" << "\n"
    << "}" << "\n";
}
