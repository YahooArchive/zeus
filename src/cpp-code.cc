/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>

#include "cpp-code.h"

void CPPCodeGenerator::constructors(Printer & p, const ir::Structure & structure) {
  const auto id = identifier(structure.identifier);

  p << id << "::" << id << "(void)";

  bool first = true;

  ir::Structure::Properties properties = structure.properties;
  std::sort(std::begin(properties), std::end(properties));

  for (const auto & property : properties) {
    std::string value;

    if (property.kind == ir::kNone) {
      if (property.type == "boolean") {
        value = "false";

      } else if (property.type == "float") {
        value = "0.0";

      } else if (property.type == "integer") {
        value = "0";

      } else if (property.type == "string") {
        value = "\"\"";
      }
    }

    if ( ! value.empty()) {
      if (first) {
        first = false;
        p << " :" << "\n";
      } else {
        p << "," << "\n";
      }
      p << tab(1) << identifier(property.property) << "(" << value << ")";
    }
  }

  p << " { }" << "\n"
    << "\n";
}

void CPPCodeGenerator::content(Printer & p, const Type::TYPES t,
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

void CPPCodeGenerator::value(Printer & p, const Value & value,
    const std::string & prefix, const int t) {
  if ( ! value.properties.empty()) {
    if (value.type == Type::kArray) {
      for (const auto & item : value.properties) {
        p << tab(t) << prefix << ".push_back(";
        content(p, item.second.type, item.second.content);
        p << ");" << "\n";
      }
    } else if (value.type == Type::kObject) {
      for (const auto & item : value.properties) {
        this->value(p, item.second, prefix + "." + item.first, t);
      }
    } else if (value.type == Type::kDynamic) {
      for (const auto & item : value.properties) {
        const std::string dynamicPrefix = prefix + "[\"" + item.first + "\"]";
        /*
        if (item.second.type == Type::kArray
            || item.second.type == Type::kDynamic) {
          p << tab(t) << dynamicPrefix << " = array();" << "\n";
        } else if (item.second.type == Type::kObject
            && ! item.second.content.empty()) {
          p << tab(t) << dynamicPrefix << " = new "
            << item.second.content << "();" << "\n";
        }
        */
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

void CPPCodeGenerator::keyDimension(Printer & p, const ir::Key & key,
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

      p << tab(t) << "case " << constantify(dimension.dimension)
        << "::" << constantify(values[item.index].first) << ":" << "\n";
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

void CPPCodeGenerator::key(Printer & p, const ir::Key & key,
    const ir::Dimensions & dimensions) {
  const std::string type = this->type(
      key.alias.empty() ? key.type : key.alias, key.kind);

  p << type << " Configuration::" << key.key << "(void) const {" << "\n";

  /*
  if (key.cache) {
    p << tab(2) << "if (isset($this->" << key.key << ")) {" << "\n"
      << tab(3) << "return $this->" << key.key << ";" << "\n"
      << tab(2) << "}" << "\n";
  }
  */

  p << tab(1) << type << " value;" << "\n";

  value(p, key.value, "value", 1);

  if (static_cast< bool >(key.dimension)) {
    keyDimension(p, key, *key.dimension, dimensions, 1);
  }

  p << tab(1) << "return value;" << "\n"
    << "}" << "\n";
}

void CPPCodeGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {

  header(p, snapshot.namespaces);

  ir::Structures structures = snapshot.structures;

  for (const auto & item : structures) {
    constructors(p, item);
  }

  tables(p, snapshot.dimensions);

  context(p, snapshot.dimensions);

  ir::Keys keys = snapshot.keys;
  std::sort(std::begin(keys), std::end(keys));

  bool first = true;

  for (const auto & key : keys) {
    if (first) {
      first = false;
    } else {
      p << "\n";
    }
    this->key(p, key, snapshot.dimensions);
  }

  footer(p, snapshot.namespaces);
}

void CPPCodeGenerator::header(Printer & p, const ir::Namespaces & n) {
  p << "#include <algorithm>" << "\n"
    << "#include <cstring>" << "\n"
    << "#include \"configuration.h\"" << "\n"
    << "\n";

  if ( ! n.empty()) {
    for (const auto & item : n) {
      p << "namespace " << item << " {" << "\n";
    }
    p << "\n";
  }
}

void CPPCodeGenerator::tables(Printer & p, const ir::Dimensions & dimensions) {
  p << "struct E {" << "\n"
    << tab(1) << "const char * const key;" << "\n"
    << tab(1) << "const uint32_t value;" << "\n"
    << "};" << "\n"
    << "\n"
    //TODO(dmorilha): can become binary search
    << "uint32_t lookup(const char * const k, const E * e, const size_t s) {" << "\n"
    << tab(1) << "for (int i = 0; i < s; ++i) {" << "\n"
    << tab(2) << "if (strcmp(e[i].key, k) == 0) {" << "\n"
    << tab(3) << "return e[i].value;" << "\n"
    << tab(2) << "}" <<"\n"
    << tab(1) << "}" << "\n"
    << tab(1) << "return 0;" << "\n"
    << "}" << "\n"
    << "\n";


  for (const auto & dimension : dimensions) {
    const std::string className = constantify(dimension.second.dimension);

    ir::DimensionEnumeration::Values values = dimension.second.values;
    values.erase(std::begin(values));
    std::sort(std::begin(values), std::end(values));

    p << "const E " << className << "_TABLE[] = {" << "\n";

    for (const auto & value : values) {
      p << tab(1) << "{\"" << value.first << "\"" << ", " << value.second << "}," << "\n";
    }

    p << "};" << "\n"
      << "\n"
      << className << "::" << className << "(const char * const s) : v(lookup(s, "
      << className << "_TABLE, " << values.size() << ")) { }" << "\n"
      << "\n";
  }
}

void CPPCodeGenerator::context(Printer & p, const ir::Dimensions & dimensions) {
  p << "struct F {" << "\n"
    << tab(1) << "const char * const key;" << "\n"
    << tab(1) << "const E * const table;" << "\n"
    << tab(1) << "const int size;" << "\n"
    << tab(1) << "bool operator < (const char * const k) const {" << "\n"
    << tab(2) << "return strcmp(key, k) < 0;" << "\n"
    << tab(1) << "}" << "\n"
    << tab(1) << "bool operator == (const char * const k) const {" << "\n"
    << tab(2) << "return strcmp(key, k) == 0;" << "\n"
    << tab(1) << "}" << "\n"
    << "};" << "\n"
    << "\n"
    << "const F DIMENSIONS[] = {" << "\n";

  for (const auto & item : dimensions) {
    const std::string & dimension = item.second.dimension;
    const std::string constant = constantify(dimension);

    p << tab(1) << "{\"" << dimension << "\", "
      << constant << "_TABLE" << ", "
      << item.second.values.size() - 1 << "}," << "\n";
  }

  p << "};" << "\n"
    << "\n"
    << "Context Context::Create(const char * * v, const int n) {" << "\n"
    << tab(1) << "Context context;" << "\n"
    << tab(1) << "uint32_t * const p = reinterpret_cast< uint32_t * >(&context);" << "\n"
    << tab(1) << "for (int i = 0; i < n - 1; i += 2) {" << "\n"
    << tab(2) << "const F * const f = std::lower_bound(DIMENSIONS, DIMENSIONS + " << dimensions.size() << ", v[i]);" << "\n"
    << tab(2) << "if (f != DIMENSIONS + " << dimensions.size() << " && *f == v[i]) {" << "\n"
    << tab(3) << "const int z = lookup(v[i + 1], f->table, f->size);" << "\n"
    << tab(3) << "if (z != 0) {" << "\n"
    << tab(4) << "p[f - DIMENSIONS] = z;" << "\n"
    << tab(4) << "continue;" << "\n"
    << tab(3) << "}" << "\n"
    << tab(2) << "}" << "\n"
    << tab(1) << "v[i] = v[i + 1] = NULL;" << "\n"
    << tab(1) << "}" << "\n"
    << tab(1) << "return context;" << "\n"
    << "};" << "\n"
    << "\n";
}

void CPPCodeGenerator::footer(Printer & p, const ir::Namespaces & n) {
  for (const auto & item : n) {
    p << "} //end of " << item << " namespace" << "\n";
  }
}

std::string CPPCodeGenerator::type(const std::string & t, const ir::Kind k) const {
  std::string result;

  //TODO(dmorilha): size is known, using vector is unecessary here.
  if (k == ir::kArray) {
    result += "std::vector< ";
  } else if (k == ir::kDynamic) {
    //TODO(dmorilha): review "map< string," choice
    result += "std::map< std::string, ";
  }

  if (t == "boolean") {
    result += "bool";

  //TODO(dmorilha): what about making this configurable?
  } else if (t == "float") {
    result += "double";

  //TODO(dmorilha): what about making this configurable?
  } else if (t == "integer") {
    result += "int64_t";

  //TODO(dmorilha): why not string or string view?
  } else if (t == "string") {
    result += "const char *";

  } else {
    result += t;
  }

  if (k == ir::kArray || k == ir::kDynamic) {
    result += " >";
  }

  return result;
}
