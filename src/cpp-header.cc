/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>

#include "cpp-header.h"

void CPPHeaderGenerator::structure (Printer & p, const ir::Structure & structure) {
  const auto id = identifier(structure.identifier);

  p << "struct " << id << " {" << "\n";

  ir::Structure::Properties properties = structure.properties;
  std::sort(std::begin(properties), std::end(properties));

  for (const auto & property : properties) {
    p << tab(1) << type(property.type, property.kind) << " "
      << identifier(property.property) << ";" << "\n";
  }

  p << "\n"
    << tab(1) << id << "(void);" << "\n"
    << "};" << "\n"
    << "\n";

  if ( ! structure.aliases.empty()) {
    for (const auto & item : structure.aliases) {
      p << "typedef " << id << " " << item << ";" << "\n";
    }
    p << "\n";
  }
}

void CPPHeaderGenerator::key(Printer & p, const ir::Key & key) {
  const std::string type = this->type(
      key.alias.empty() ? key.type : key.alias, key.kind);
  p << tab(1) << type << " " << key.key << "(void) const;" << "\n";
}

void CPPHeaderGenerator::contextClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "struct Context {" << "\n";

  {
    const ir::Dimensions & dimensions = snapshot.dimensions;

    for (const auto & dimension : dimensions) {
      p << tab(1) << constantify(dimension.first) << " "
        << identifier(dimension.first) << ";" << "\n";
    }
  }


  p << "\n"
    << tab(1) << "static Context Create(const char * *, const int);" << "\n"
    << "};" << "\n"
    << "\n";
}

void CPPHeaderGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {

  p << "struct Configuration {" << "\n"
    << tab(1) << "const Context context;" << "\n"
    << "\n"
    << tab(1) << "Configuration(const Context & c) : context(c) { }" << "\n"
    << "\n";

  ir::Keys keys = snapshot.keys;
  std::sort(std::begin(keys), std::end(keys));

  for (const auto & key : keys) {
    this->key(p, key);
  }

  p << "};" << "\n"
    << "\n";
}

void CPPHeaderGenerator::dimension(Printer & p, const ir::DimensionEnumeration & dimension) {

  const std::string className = constantify(dimension.dimension);

  ir::DimensionEnumeration::Values values = dimension.values;
  std::sort(std::begin(values), std::end(values));

  p << "struct " << className << " {" << "\n"
    << tab(1) << "enum ENUM {" << "\n";

  for (const auto & value : values) {
    p << tab(2) << constantify(value.first) << " = " << value.second << "," << "\n";
  }

  p << tab(1) << "};" << "\n"
    << tab(1) << "static uint32_t size(void) { return " << values.size() << "; }" << "\n"
    << tab(1) << "D(" << className << ");" << "\n"
    << "};" << "\n"
    << "\n";
}

void CPPHeaderGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {
  header(p, snapshot.namespaces);

  p << "#define D(TypeName)" << " \\" << "\n"
    << tab(1) << "TypeName(const char * const);" << " \\" << "\n"
    << tab(1) << "TypeName(const ENUM e = NONE) : v(e) { }" << " \\" << "\n"
    << tab(1) << "TypeName & operator = (const ENUM e) {" << " \\" << "\n"
    << tab(2) << "v = e;" << " \\" << "\n"
    << tab(2) << "return *this;" << " \\" << "\n"
    << tab(1) << "}" << " \\" << "\n"
    << tab(1) << "operator ENUM (void) const {" << " \\" << "\n"
    << tab(2) << "return static_cast< ENUM >(v);" << " \\" << "\n"
    << tab(1) << "}" << " \\" << "\n"
    << "private:" << " \\" << "\n"
    << tab(1) << "uint32_t v" << "\n"
    << "\n";

  for (const auto & dimension : snapshot.dimensions) {
    this->dimension(p, dimension.second);
  }

  p << "#undef D" << "\n"
    << "\n";

  contextClass(p, snapshot);

  {
    ir::Structures structures = snapshot.structures;

    for (const auto & item : structures) {
      structure(p, item);
    }
  }

  configurationClass(p, snapshot);

  footer(p, snapshot.namespaces);
}

void CPPHeaderGenerator::header(Printer & p, const ir::Namespaces & n) {
  p << "#ifndef CONFIGURATION_H" << "\n"
    << "#define CONFIGURATION_H" << "\n"
    << "\n"
    << "#include <map>" << "\n"
    << "#include <vector>" << "\n"
    << "#include <stdint.h>" << "\n"
    << "#include <string>" << "\n"
    << "\n";

  if ( ! n.empty()) {
    for (const auto & item : n) {
      p << "namespace " << item << " {" << "\n";
    }
    p << "\n";
  }
}

void CPPHeaderGenerator::footer(Printer & p, const ir::Namespaces & n) {
  for (const auto & item : n) {
    p << "} //end of " << item << " namespace" << "\n";
  }

  p << "#endif //CONFIGURATION_H";
}

std::string CPPHeaderGenerator::type(const std::string & t, const ir::Kind k) const {
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
