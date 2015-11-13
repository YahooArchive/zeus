/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>

#include "java.h"

void JavaGenerator::structure (Printer & p, const ir::Structure & structure) {
  const auto id = identifier(structure.identifier);

  p << "class " << id << " {" << "\n";

  {
    bool constructor = false;

    ir::Structure::Properties properties = structure.properties;
    std::sort(std::begin(properties), std::end(properties));

    for (const auto & property : properties) {
      p << tab(1) << type(property.type, property.kind) << " "
        << identifier(property.property) << ";" << "\n";
    }

    if (constructor) {
      p << "\n"
        << tab(1) << id << "() { }" << "\n";
    }

    p << "}" << "\n"
      << "\n";
  }
}

void JavaGenerator::content(Printer & p, const Type::TYPES t,
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
    break;

  case Type::kString:
    p << '"' << c << '"'; break;

  case Type::kUndefined:
    //std::cerr << "type is undefined" << std::endl;
    break;

  default:
    //std::cerr << "ignoring unknown type" << std::endl;
    break;
  }
}

void JavaGenerator::value(Printer & p, const Value & value,
    const std::string & prefix, const int t) {
  if ( ! value.properties.empty()) {
    if (value.type == Type::kArray) {
      for (const auto & item : value.properties) {
        p << tab(t) << prefix << ".add(";
        content(p, item.second.type, item.second.content);
        p << ");" << "\n";
      }
    } else if (value.type == Type::kObject) {
      for (const auto & item : value.properties) {
        this->value(p, item.second, prefix + "." + item.first, t);
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

void JavaGenerator::keyDimension(Printer & p, const ir::Key & key,
    const ir::Dimension & dimension, const ir::Dimensions & dimensions,
    const int t) {

  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "switch (context." << identifier(dimension.dimension) << ") {" << "\n";
  } else {
    assert(dimension.values.size() == 1 ||
        (dimension.values.empty() && dimension.next != nullptr));
    p << tab(t) << "//skipping dimension " << dimension.dimension << "\n";
  }

  for (const auto & item : dimension.values) {
    assert(item.index > 0);

    if ( ! dimension.skip) {
      const auto & iterator = dimensions.find(dimension.dimension);
      assert(iterator != dimensions.end());
      const auto & values = iterator->second.values;
      assert(values.size() > item.index);

      p << tab(t) << "case " << constantify(values[item.index].first)
        << ":" << "\n";
    }

    value(p, item.value, "value", t + (dimension.skip ? 0 : 1));

    if (static_cast< bool >(item.dimension)) {
      keyDimension(p, key, *item.dimension, dimensions,
          t + (dimension.skip ? 0 : 1));
    }

    if ( ! dimension.skip) {
      p << tab(t + 1) << "break;" << "\n";
    }
  }

  if (static_cast< bool >(dimension.next)) {
    if (dimension.skip || dimension.values.empty()) {
      keyDimension(p, key, *dimension.next, dimensions, t);
    } else {
      p << tab(t) << "default:" << "\n";
      keyDimension(p, key, *dimension.next, dimensions, t + 1);
      p << tab(t + 1) << "break;" << "\n";
    }
  }

  if ( ! (dimension.skip || dimension.values.empty())) {
    p << tab(t) << "}" << "\n";
  }
}

void JavaGenerator::key(Printer & p, const ir::Key & key,
    const ir::Dimensions & dimensions) {
  const std::string type = this->type(key.type, key.kind);

  p << tab(1) << type << " " << key.key << "() {" << "\n";

  if (key.cache) { }

  p << tab(2) << type << " value = new " << type << "();" << "\n";

  if ( ! (key.value.properties.empty() && key.value.content.empty())) {
    value(p, key.value, "value", 2);
  }

  if (static_cast< bool >(key.dimension)) {
    keyDimension(p, key, *key.dimension, dimensions, 2);
  }

  p << tab(2) << "return value;" << "\n"
    << tab(1) << "}" << "\n"
    << "\n";
}

void JavaGenerator::contextClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "class Context {" << "\n";

  {
    const ir::Dimensions & dimensions = snapshot.dimensions;

    for (const auto & dimension : dimensions) {
      p << tab(1) << constantify(dimension.first) << " "
        << identifier(dimension.first) << ";" << "\n";
    }

    p << "\n"
      << tab(1) << "Context() {" << "\n";

    for (const auto & dimension : dimensions) {
      p << tab(2) << identifier(dimension.first) << " = "
        << constantify(dimension.first) << ".NONE;" << "\n";
    }

    p << tab(1) << "}" << "\n";
  }

  p << "}" << "\n"
    << "\n";
}

void JavaGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "public class Configuration {" << "\n"
    << tab(1) << "final Context context;" << "\n"
    << "\n"
    << tab(1) << "Configuration(Context c) {" << "\n"
    << tab(2) << "this.context = c;" << "\n"
    << tab(1) << "}" << "\n"
    << "\n";

  {
    ir::Keys keys = snapshot.keys;
    std::sort(std::begin(keys), std::end(keys));

    p << tab(1) << "//print each configuration key" << "\n";

    for (const auto & key : keys) {
      this->key(p, key, snapshot.dimensions);
    }
  }

  p << "};" << "\n"
    << "\n";
}

void JavaGenerator::dimension(Printer & p, const ir::DimensionEnumeration & dimension) {

  const std::string enumerationName = constantify(dimension.dimension);

  ir::DimensionEnumeration::Values values = dimension.values;
  std::sort(std::begin(values), std::end(values));

  p << "enum " << enumerationName << " {" << "\n";

  if ( ! values.empty()) {
    bool first = true;
    for (const auto & value : values) {
      if (first) {
        first = false;
      } else {
        p << "," << "\n";
      }
      p << tab(1) << constantify(value.first) << "(" << value.second << ")";
    }
    p << ";" << "\n";
  }

  p << tab(1) << "private final int value;" << "\n"
    << tab(1) << enumerationName << "(int value) {" << "\n"
    << tab(2) << "this.value = value;" << "\n"
    << tab(1) << "}" << "\n"
    << "}" << "\n"
    << "\n";

  /*
  p << "\n"
    << tab(1) << "//print look-up table" << "\n"
    << tab(1) << "static $table = array(" << "\n";

  for (const auto & value : values) {
    p << tab(2) << "'" << value.first << "' => " << className << "::" << constantify(value.first) << "," << "\n";
  }

  p << tab(1) << ");" << "\n"
    << "}" << "\n"
    << "\n";
   */
}

void JavaGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {
  header(p, snapshot.namespaces);

  {
    ir::Structures structures = snapshot.structures;

    for (const auto & item : structures) {
      structure(p, item);
    }
  }

  for (const auto & dimension : snapshot.dimensions) {
    this->dimension(p, dimension.second);
  }

  contextClass(p, snapshot);

  configurationClass(p, snapshot);
}

void JavaGenerator::header(Printer & p, const ir::Namespaces & n) {
  if ( ! n.empty()) {
    bool first = true;
    p << "package ";
    for (const auto & item : n) {
      p << item;
      if (first) {
        first = false;
      } else {
        p << ".";
      }
    }
    p << ";" << "\n"
      "\n";
  }

  p << "import java.util.ArrayList;" << "\n"
    << "import java.util.HashMap;" << "\n"
    << "\n";
}

std::string JavaGenerator::type(const std::string & t, const ir::Kind k) const {
  std::string result;

  //TODO(dmorilha): size is known, using vector is unecessary here.
  if (k == ir::kArray) {
    result += "ArrayList< ";
  } else if (k == ir::kDynamic) {
    result += "HashMap< String, ";
  }

  if (t == "boolean") {
    result += "boolean";

  //TODO(dmorilha): what about making this configurable?
  } else if (t == "float") {
    result += "double";

  //TODO(dmorilha): what about making this configurable?
  } else if (t == "integer") {
    result += "long";

  } else if (t == "string") {
    result += "String";

  } else {
    result += t;
  }

  if (k == ir::kArray
      || k == ir::kDynamic) {
    result += " >";
  }

  return result;
}
