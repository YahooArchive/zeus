/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>

#include <assert.h>

#include "php.h"

void PHPGenerator::structure (Printer & p, const ir::Structure & structure) {
  p << "class " << identifier(structure.identifier) << " extends ConfigurationStructure {" << "\n";

  ir::Structure::Properties properties = structure.properties;
  std::sort(std::begin(properties), std::end(properties));

  for (const auto & property : properties) {
    p << tab(1) << "public $" << identifier(property.property) << ";";

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

  p << "\n"
    << tab(1) << "public function __construct() {" << "\n";

  for (const auto & property : properties) {
    p << tab(2) << "$this->" << identifier(property.property) << " = ";

    if (property.kind == ir::kArray
        || property.kind == ir::kDynamic) {
      p << "array()";

    } else if (property.type == "boolean") {
      p << "false";

    } else if (property.type == "float"
        || property.type == "integer") {
      p << "0";

    } else if (property.type == "string") {
      p << "''";

    } else {
      p << "new " << identifier(property.type) << "()";
    }

    p << ";" << "\n";
  }

  p << tab(1) << "}" << "\n"
    << "}" << "\n"
    << "\n";
}

void PHPGenerator::content(Printer & p, const Type::TYPES t,
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

void PHPGenerator::value(Printer & p, const Value & value,
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
        p << tab(t) << prefix << "[] = ";
        content(p, item.second.type, item.second.content);
        p << ";" << "\n";
      }
    } else if (value.type == Type::kObject) {
      for (const auto & item : value.properties) {
        this->value(p, item.second, prefix + "->" + item.first, t);
      }
    } else if (value.type == Type::kDynamic) {
      for (const auto & item : value.properties) {
        const std::string dynamicPrefix = prefix + "['" + item.first + "']";
        if (item.second.type == Type::kArray
            || item.second.type == Type::kDynamic) {
          p << tab(t) << dynamicPrefix << " = array();" << "\n";
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

void PHPGenerator::keyDimension(Printer & p, const ir::Key & key,
    const ir::Dimension & dimension, const int t) {

  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "switch ($this->" << identifier(dimension.dimension) << ") {" << "\n";
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

    value(p, item.value, "$value", t + (dimension.skip ? 0 : 1));

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

void PHPGenerator::key(Printer & p, const ir::Key & key) {
  p << tab(1) << "public function " << key.key << "() {" << "\n";

  if (key.cache) {
    p << tab(2) << "if (array_key_exists('" << key.key << "', $this->cache)) {" << "\n"
      << tab(3) << "return $this->" << key.key << ";" << "\n"
      << tab(2) << "}" << "\n";
  }

  if (key.value.type == Type::kArray
      || key.value.type == Type::kDynamic) {
    p << tab(2) << "$value = array();" << "\n";
  } else if ( ! nativeType(key.type)) {
    p << tab(2) << "$value" << " = new " << key.type
      << "()" << ";" << "\n";
  }

  value(p, key.value, "$value", 2);

  if (static_cast< bool >(key.dimension)) {
    keyDimension(p, key, *key.dimension, 2);
  }

  if (key.cache) {
    p << tab(2) << "$this->cache['" << key.key << "'] = true;" << "\n"
      << tab(2) << "$this->" << key.key << " = $value;" << "\n";
  }

  p << tab(2) << "return $value;" << "\n"
    << tab(1) << "}" << "\n"
    << "\n";
}

void PHPGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "class Configuration {" << "\n"
    << tab(1) << "private $cache = array();" << "\n"
    << "\n"
    << tab(1) << "//print private data members" << "\n";

  {
    const ir::Dimensions & dimensions = snapshot.dimensions;

    for (const auto & dimension : dimensions) {
      p << tab(1) << "private $" << identifier(dimension.first) << ";" << "\n";
    }

    p << "\n"
      << tab(1) << "public function __construct($dimensions) {" << "\n"
      << tab(2) << "//print data member initialization" << "\n";

    for (const auto & dimension : dimensions) {
      p << tab(2) << "$this->" << identifier(dimension.first) << " = "
          << "array_key_exists('" << dimension.first << "', $dimensions) ? "
          << constantify(dimension.first) << "::$table[$dimensions['"
          << dimension.first << "']] : 0;" << "\n";
    }

    p << tab(1) << "}" << "\n"
      << "\n";
  }

  p << tab(1) << "public function __get($key) {" << "\n"
    << tab(2) << "return $this->{\"$key\"}();" << "\n"
    << tab(1) << "}" << "\n"
    << "\n";

  p << tab(1) << "public function __isset($key) {" << "\n"
    << tab(2) << "return method_exists($this, $key);" << "\n"
    << tab(1) << "}" << "\n"
    << "\n";

  {
    ir::Keys keys = snapshot.keys;
    std::sort(std::begin(keys), std::end(keys));

    p << tab(1) << "//print each configuration key" << "\n";

    for (const auto & key : keys) {
      this->key(p, key);
    }
  }

  p << "}" << "\n";
}

void PHPGenerator::dimension(Printer & p, const ir::DimensionEnumeration & dimension) {

  const std::string className = constantify(dimension.dimension);

  ir::DimensionEnumeration::Values values = dimension.values;
  std::sort(std::begin(values), std::end(values));

  p << "//print each dimension class" << "\n"
    << "\n"
    << "class " << className << " {" << "\n"
    << tab(1) << "//print enumeration" << "\n";

  for (const auto & value : values) {
    p << tab(1) << "const " << constantify(value.first) << " = " << value.second << ";" << "\n";
  }

  p << "\n"
    << tab(1) << "//print look-up table" << "\n"
    << tab(1) << "static $table = array(" << "\n";

  for (const auto & value : values) {
    p << tab(2) << "'" << value.first << "' => " << className << "::" << constantify(value.first) << "," << "\n";
  }

  p << tab(1) << ");" << "\n"
    << "}" << "\n"
    << "\n";
}

void PHPGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {
  header(p, snapshot.namespaces);

  {
    ir::Structures structures = snapshot.structures;

    if ( ! structures.empty()) {
      //php allows declarings classes in any sequence.
      std::sort(std::begin(structures), std::end(structures));

      p << "class ConfigurationStructure extends \\ArrayObject {" << "\n"
        << tab(1) << "public function offsetGet($k) {" << "\n"
        << tab(2) << "return $this->{$k};" << "\n"
        << tab(1) << "}" << "\n"
        << "}" << "\n"
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
}

void PHPGenerator::header(Printer & p, const ir::Namespaces & n) {
  p << "<?php" << "\n";

  if ( ! n.empty()) {
    bool first = true;
    p << "namespace ";
    for (const auto & item : n) {
      if (first) {
        first = false;
      } else {
        p << "\\";
      }
      p << item;
    }
    p << ";" << "\n";
  }

  p << "\n";
}
