/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>

#include "cpp-json-header.h"

void CPPJsonHeaderGenerator::key(Printer & p, const ir::Key & key) {
  const std::string type = this->type(key.type, key.kind);
  p << tab(1) << "std::string & " << key.key << "(std::string &);" << "\n";
}

void CPPJsonHeaderGenerator::configurationClass(Printer & p, const ir::Snapshot & snapshot) {
  p << "struct ConfigurationJson {" << "\n"
    << tab(1) << "Configuration configuration_;" << "\n"
    << "\n"
    << tab(1) << "ConfigurationJson(Context & c)"
    << " : configuration_(c) { }" << "\n"
    << "\n";

  ir::Keys keys = snapshot.keys;
  std::sort(std::begin(keys), std::end(keys));

  p << tab(1) << "//print each configuration key" << "\n";

  for (const auto & key : keys) {
    this->key(p, key);
  }

  p << tab(1) << "std::string & keys(const char * *, const int, std::string &);" << "\n"
    << tab(1) << "std::string & all(std::string &);" << "\n"
    << "};" << "\n"
    << "\n";
}

void CPPJsonHeaderGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {
  header(p, snapshot.namespaces);
  configurationClass(p, snapshot);
  footer(p, snapshot.namespaces);
}

void CPPJsonHeaderGenerator::header(Printer & p, const ir::Namespaces & n) {
  p << "#ifndef CONFIGURATION_JSON_H" << "\n"
    << "#define CONFIGURATION_JSON_H" << "\n"
    << "\n"
    << "#include <string>" << "\n"
    << "\n"
    << "#include \"configuration.h\"" << "\n"
    << "\n";

  if ( ! n.empty()) {
    for (const auto & item : n) {
      p << "namespace " << item << " {" << "\n";
    }
    p << "\n";
  }
}

void CPPJsonHeaderGenerator::footer(Printer & p, const ir::Namespaces & n) {
  for (const auto & item : n) {
    p << "} //end of " << item << " namespace" << "\n";
  }

  p << "\n"
    << "extern \"C\" {" << "\n"
    << "void json(const char * *, const int, const char * *," << "\n"
    << tab(4) << "const int, char * *, int *);" << "\n"
    << "int version(void);" << "\n"
    << "}" << "\n"
    << "\n"
    << "#endif //CONFIGURATION_JSON_H";
}

std::string CPPJsonHeaderGenerator::type(const std::string & t, const bool a) const {
  std::string result;

  //TODO(dmorilha): size is known, using vector is unecessary here.
  if (a) {
    result += "std::vector< ";
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

  if (a) {
    result += " >";
  }

  return result;
}
