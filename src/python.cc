/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>

#include <assert.h>

#include "python.h"

void PythonGenerator::structure (Printer & p, const ir::Structure & structure, const ir::Namespaces & n) {
  const std::string id = identifier(structure.identifier);

  p << "class " << id << ":" << "\n";

  ir::Structure::Properties properties = structure.properties;
  std::sort(std::begin(properties), std::end(properties));

  for (const auto & property : properties) {
    p << tab(1) << identifier(property.property) << " = None;";

    if ( ! property.comments.declaration.empty()) {
      p << " # " << property.comments.declaration;
    } else if ( ! property.type.empty()) {
      p << " # type: " << property.type;
      if (property.kind == ir::kArray) {
        p << " (array)";
      } else if (property.kind == ir::kDynamic) {
        p << " (dynamic)";
      }
    }

    p << "\n";
  }

  p << "\n"
    << tab(1) << "def __init__(self):" << "\n";

  for (const auto & property : properties) {
    p << tab(2) << "self." << identifier(property.property) << " = ";

    if (property.kind == ir::kDynamic) {
      p << "{}";

    } else if (property.kind == ir::kArray) {
      p << "[]";

    } else if (property.type == "boolean") {
      p << "false";

    } else if (property.type == "float"
        || property.type == "integer") {
      p << "0";

    } else if (property.type == "string") {
      p << "''";

    } else {
      p << identifier(property.type) << "()";
    }

    p << "\n";
  }

  /*
  if ( ! structure.aliases.empty()) {
    std::string ns;
    for (const auto & item : n) {
      ns += item + "\\";
    }
    for (const auto & item : structure.aliases) {
      p << "class_alias('" << ns << id << "', '"
        << ns << item << "');" << "\n";
    }
  }
  */

  p << "\n";
}

void PythonGenerator::content(Printer & p, const Type::TYPES t,
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
    p << " " << c << "()";
    break;

  case Type::kString:
    p << "'" << c << "'";
    break;

  case Type::kUndefined:
    //std::cerr << "type is undefined" << std::endl;
    break;

  default:
    //std::cerr << "ignoring unknown type" << std::endl;
    break;
  }
}

void PythonGenerator::value(Printer & p, const Value & value,
    const std::string & prefix, const int t) {

  /*
  if (value.reset) {
    if (value.array) {
      p << tab(t) << prefix << " = array();" << "\n";
    }
  }
  */

  if ( ! value.properties.empty()) {
    if (value.type == Type::kArray) {
      for (const auto & item : value.properties) {
        if (item.second.ignore) {
          continue;
        }
        p << tab(t) << prefix << ".append(";
        content(p, item.second.type, item.second.content);
        p << ")" << "\n";
      }
    } else if (value.type == Type::kObject) {
      for (const auto & item : value.properties) {
        assert( ! item.second.ignore);
        this->value(p, item.second, prefix + "." + item.first, t);
      }
    } else if (value.type == Type::kDynamic) {
      for (const auto & item : value.properties) {
        if (item.second.ignore) {
          continue;
        }
        const std::string dynamicPrefix = prefix + "['" + item.first + "']";
        if (item.second.type == Type::kDynamic) {
          p << tab(t) << dynamicPrefix << " = {}" << "\n";
        } else if (item.second.type == Type::kArray) {
          p << tab(t) << dynamicPrefix << " = []" << "\n";
        } else if (item.second.type == Type::kObject
            && ! item.second.content.empty()) {
          p << tab(t) << dynamicPrefix << " = "
            << item.second.content << "()" << "\n";
        }
        this->value(p, item.second, dynamicPrefix, t);
      }
    } else {
      assert(false);
    }
  } else if (value.type != Type::kUndefined) {
    assert( ! value.content.empty() || value.type == Type::kString);
    assert( ! value.ignore);
    p << tab(t) << prefix << " = ";
    content(p, value.type, value.content);
    p << "\n";
  }
}

void PythonGenerator::keyDimension(Printer & p, const ir::Key & key,
    const ir::Dimension & dimension, const ir::Dimensions & dimensions,
    const int t) {

  /*
  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "switch (self." << identifier(dimension.dimension) << "):" << "\n";
  } else {
    assert(dimension.values.size() == 1 ||
        (dimension.values.empty() && dimension.next != nullptr));
    p << tab(t) << "# skipping dimension " << dimension.dimension << "\n";
  }
  */

  bool first = true;

  const std::string id = identifier(dimension.dimension);

  for (const auto & item : dimension.values) {
    assert(item.index > 0);

    if ( ! dimension.skip) {
      if ( ! first) {
        p << "\n"
          << tab(t) << "elif ";
      } else {
        p << tab(t) << "if ";
        first = false;
      }

      const auto & iterator = dimensions.find(dimension.dimension);
      assert(iterator != dimensions.end());
      const auto & values = iterator->second.values;
      assert(values.size() > item.index);

      p << "self." << id << " == " << item.index << ": # "
        << values[item.index].first << "\n";
    }

    value(p, item.value, "value", t + (dimension.skip ? 0 : 1));

    if (static_cast< bool >(item.dimension)) {
      keyDimension(p, key, *item.dimension, dimensions,
          t + (dimension.skip ? 0 : 1));
    }

    /*
    if ( ! dimension.skip) {
      p << "\n";
    }
    */
  }

  if (static_cast< bool >(dimension.next)) {
    if (dimension.skip || dimension.values.empty()) {
      keyDimension(p, key, *dimension.next, dimensions, t);
    } else {
      p << "\n"
        << tab(t) << "else:" << "\n";
      keyDimension(p, key, *dimension.next, dimensions, t + 1);
      p << "\n";
    }
  }

  /*
  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "\n";
  }
  */
}

void PythonGenerator::key(Printer & p, const ir::Key & key,
    const ir::Dimensions & dimensions) {
  p << tab(1) << "def " << key.key << "(self):" << "\n";

  /*
  if (key.cache) {
    p << tab(2) << "if (array_key_exists('" << key.key << "', $this->cache)) {" << "\n"
      << tab(3) << "return $this->" << key.key << ";" << "\n"
      << tab(2) << "}" << "\n";
  }
  */

  if (key.value.type == Type::kDynamic) {
    p << tab(2) << "value = {}" << "\n";
  } else if (key.value.type == Type::kArray) {
    p << tab(2) << "value = []" << "\n";
  } else if ( ! key.alias.empty()) {
    p << tab(2) << "value" << " = " << key.alias
      << "()" << "\n";
  } else if ( ! nativeType(key.type)) {
    p << tab(2) << "value" << " = " << key.type
      << "()" << "\n";
  }

  value(p, key.value, "value", 2);

  if (static_cast< bool >(key.dimension)) {
    keyDimension(p, key, *key.dimension, dimensions, 2);
  }

  /*
  if (key.cache) {
    p << tab(2) << "$this->cache['" << key.key << "'] = true;" << "\n"
      << tab(2) << "$this->" << key.key << " = $value;" << "\n";
  }
  */

  p << tab(2) << "return value" << "\n"
    << "\n";
}

void PythonGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "class Configuration:" << "\n"
    << tab(1) << "# print private data members" << "\n";

  {
    const ir::Dimensions & dimensions = snapshot.dimensions;

    for (const auto & dimension : dimensions) {
      p << tab(1) << identifier(dimension.first) << " = 0\n";
    }

    p << "\n"
      << tab(1) << "def __init__(self, dimensions):" << "\n"
      << tab(2) << "# print data member initialization" << "\n";

    for (const auto & dimension : dimensions) {
      const std::string id = identifier(dimension.first);

      p << tab(2) << "value = dimensions.get('" << id << "', 0)" << "\n"
        << tab(2) << "self." << id << " = value if isinstance(value, int) else "
        << constantify(dimension.first) << ".table.get(value, 0)" << "\n"
        << "\n";
    }
  }

  {
    ir::Keys keys = snapshot.keys;
    std::sort(std::begin(keys), std::end(keys));

    p << tab(1) << "# print each configuration key" << "\n";

    for (const auto & key : keys) {
      this->key(p, key, snapshot.dimensions);
    }
  }

  p << "\n";
}

void PythonGenerator::dimension(Printer & p, const ir::DimensionEnumeration & dimension) {

  const std::string className = constantify(dimension.dimension);

  ir::DimensionEnumeration::Values values = dimension.values;
  std::sort(std::begin(values), std::end(values));

  p << "# print each dimension class" << "\n"
    << "\n"
    << "class " << className << ":" << "\n"
    << tab(1) << "# print enumeration" << "\n";

  for (const auto & value : values) {
    p << tab(1) << constantify(value.first) << " = " << value.second << "\n";
  }

  p << "\n"
    << tab(1) << "# print look-up table" << "\n"
    << tab(1) << "table = {" << "\n";

  for (const auto & value : values) {
    p << tab(2) << "'" << value.first << "' : " << constantify(value.first) << "," << "\n";
  }

  p << tab(1) << "}" << "\n"
    << "\n";
}

void PythonGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {
  header(p, snapshot.namespaces);

  {
    ir::Structures structures = snapshot.structures;

    std::sort(std::begin(structures), std::end(structures));

    for (const auto & item : structures) {
      structure(p, item, snapshot.namespaces);
    }
  }

  for (const auto & dimension : snapshot.dimensions) {
    this->dimension(p, dimension.second);
  }

  configurationClass(p, snapshot);
}

void PythonGenerator::header(Printer & p, const ir::Namespaces & n) {
  /*
  p << "from enum import Enum" << "\n"
    << "\n";
    */
}
