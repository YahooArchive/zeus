/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <algorithm>
#include <assert.h>

#include "cpp-json-code.h"

void CPPJsonCodeGenerator::content(Printer & p, const Structures & s,
    const std::string & t, const ir::Kind & k, const std::string & v,
    const int ta) {

  switch (k) {
  case ir::kNone:
    break;

  case ir::kArray:
    p << ";" << "\n"
      << tab(ta) << "{" << "\n"
      << tab(ta + 1) << "const " << type(t, k) << "::const_iterator" << "\n"
      << tab(ta + 2) << "begin = " << v << ".begin()," << "\n"
      << tab(ta + 2) << "end = " << v << ".end();" << "\n"
      << tab(ta + 1) << type(t, k) << "::const_iterator it = begin;" << "\n"
      << tab(ta + 1) << "ss << \"[\";" << "\n"
      << tab(ta + 1) << "for (; it != end; ++it) {" << "\n"
      << tab(ta + 2) << "if (it != begin) {" << "\n"
      << tab(ta + 3) << "ss << \",\";" << "\n"
      << tab(ta + 2) << "}" << "\n"
      << tab(ta + 2) << "ss ";

    content(p, s, t, ir::kNone, "(*it)", ta + 2);

    p << ";" << "\n"
      << tab(ta + 1) << "}" << "\n"
      << tab(ta + 1) << "ss << \"]\";" << "\n"
      << tab(ta) << "}" << "\n"
      << tab(ta) << "ss ";
    return;

  case ir::kDynamic:
    p << ";" << "\n"
      << tab(ta) << "{" << "\n"
      << tab(ta + 1) << "const " << type(t, k) << "::const_iterator" << "\n"
      << tab(ta + 2) << "begin = " << v << ".begin()," << "\n"
      << tab(ta + 2) << "end = " << v << ".end();" << "\n"
      << tab(ta + 1) << type(t, k) << "::const_iterator it = begin;" << "\n"
      << tab(ta + 1) << "ss << \"{\";" << "\n"
      << tab(ta + 1) << "for (; it != end; ++it) {" << "\n"
      << tab(ta + 2) << "if (it != begin) {" << "\n"
      << tab(ta + 3) << "ss << \",\";" << "\n"
      << tab(ta + 2) << "}" << "\n"
      << tab(ta + 2) << "ss << \"\\\"\" << it->first << \"\\\":\" ";

    content(p, s, t, ir::kNone, "it->second", ta + 2);

    p << ";" << "\n"
      << tab(ta + 1) << "}" << "\n"
      << tab(ta + 1) << "ss << \"}\";" << "\n"
      << tab(ta) << "}" << "\n"
      << tab(ta) << "ss ";
    return;

  default:
    assert(false); //UNRECHEABLE
  }

  if (nativeType(t)) {
    if (t == "string") {
      p << "<< \"\\\"\" << " << v << " << \"\\\"\"";
    } else if (t == "boolean") {
      p << "<< ("<< v << " ? \"true\" : \"false\")";
    } else {
      p << "<< " << v;
    }
  } else {
    const Structures::const_iterator it = s.find(t);
    assert(it != s.end());
    p << "<< \"{\"" << "\n";
    if ( ! it->second.properties.empty()) {
      bool first = true;
      for (const auto & item : it->second.properties) {
        if (first) {
          first = false;
        } else {
          p << " << \",\"" << "\n";
        }
        p << tab(ta + 1) << "<< \"\\\"" << item.property << "\\\":\" ";
        content(p, s, item.type, item.kind, v + "." + item.property, ta);
      }
      p << "\n";
    }
    p << tab(ta + 1) << "<< \"}\"";
  }
}

void CPPJsonCodeGenerator::all(Printer & p, const ir::Keys & keys) {
  p << "typedef std::string & (ConfigurationJson::* Pointer) (std::string &);" << "\n"
    << "\n"
    << "struct E {" << "\n"
    << tab(1) << "const char * const key;" << "\n"
    << tab(1) << "const Pointer pointer;" << "\n"
    << tab(1) << "bool operator < (const char * const k) const {" << "\n"
    << tab(2) << "return strcmp(key, k) < 0;" << "\n"
    << tab(1) << "}" << "\n"
    << tab(1) << "bool operator == (const char * const k) const {" << "\n"
    << tab(2) << "return strcmp(key, k) == 0;" << "\n"
    << tab(1) << "}" << "\n"
    << "};" << "\n"
    << "\n"
    << "const E KEYS[] = {" << "\n";

  for (const auto & key : keys) {
    p << tab(1) << "{\"" << key.key << "\"" << ", &ConfigurationJson::" << key.key << "}," << "\n";
  }

  p << "};" << "\n"
    << "\n"
    << "std::string & ConfigurationJson::keys(const char * * k, const int s, std::string & o) {" << "\n"
    << tab(1) << "o += \"{\";" << "\n"
    << tab(1) << "bool first = true;" << "\n"
    << tab(1) << "for (int i = 0; i < s; ++i) {" << "\n"
    << tab(2) << "if (k[i] != NULL) {" << "\n"
    << tab(3) << "const E * const e = std::lower_bound(KEYS, KEYS + " << keys.size() << ", k[i]);" << "\n"
    << tab(3) << "if (e != KEYS + " << keys.size() << " && *e == k[i]) {" << "\n"
    << tab(4) << "if (first) {" << "\n"
    << tab(5) << "first = false;" << "\n"
    << tab(4) << "} else {" << "\n"
    << tab(5) << "o += \",\";" << "\n"
    << tab(4) << "}" << "\n"
    << tab(4) << "o += \"\\\"\";" << "\n"
    << tab(4) << "o += e->key;" << "\n"
    << tab(4) << "o += \"\\\":\";" << "\n"
    << tab(4) << "(this->*(e->pointer))(o);" << "\n"
    << tab(3) << "} else {" << "\n"
    << tab(4) << "k[i] = NULL;" << "\n"
    << tab(3) << "}" << "\n"
    << tab(2) << "}" << "\n"
    << tab(1) << "}" << "\n"
    << tab(1) << "o += \"}\";" << "\n"
    << tab(1) << "return o;" << "\n"
    << "}" << "\n"
    << "\n"
    << "std::string & ConfigurationJson::all(std::string & o) {" << "\n"
    << tab(1) << "o += \"{\";" << "\n";

  if (keys.size() > 0) {
    p << tab(1) << "o += \"\\\"\";" << "\n"
      << tab(1) << "o += KEYS[0].key;" << "\n"
      << tab(1) << "o += \"\\\":\";" << "\n"
      << tab(1) << "(this->*KEYS[0].pointer)(o);" << "\n"
      << tab(1) << "for (int i = 1; i < " << keys.size() << "; ++i) {" << "\n"
      << tab(2) << "o += \",\\\"\";" << "\n"
      << tab(2) << "o += KEYS[i].key;" << "\n"
      << tab(2) << "o += \"\\\":\";" << "\n"
      << tab(2) << "(this->*KEYS[i].pointer)(o);" << "\n"
      << tab(2) << "}" << "\n";
  }

  p << tab(1) << "o += \"}\";" << "\n"
    << tab(1) << "return o;" << "\n"
    << "}" << "\n";
}

void CPPJsonCodeGenerator::key(Printer & p, const ir::Key & key,
    const Structures & s) {
  const std::string type = this->type(key.type, key.kind);

  p << "std::string & ConfigurationJson::" << key.key << "(std::string & o) {" << "\n";

  p << tab(1) << type << " value = configuration_."
    << key.key << "();" << "\n"
    << tab(1) << "std::stringstream ss;" << "\n";

  p << tab(1) << "ss ";

  content(p, s, key.type, key.kind, "value", 1);

  p << ";" << "\n"
    << tab(1) << "o += ss.str();" << "\n"
    << tab(1) << "return o;" << "\n"
    << "}" << "\n";
}

void CPPJsonCodeGenerator::generate(Printer & p, const ir::Snapshot & snapshot) {

  ir::Keys keys = snapshot.keys;
  std::sort(std::begin(keys), std::end(keys));

  const bool hasKeys = std::find(std::begin(keys),
      std::end(keys), "keys") != std::end(keys);

  header(p, snapshot.namespaces, hasKeys);

  Structures structures;

  for (const auto & item : snapshot.structures) {
    structures.insert(make_pair(item.identifier, item));
  }

  for (const auto & key : keys) {
    this->key(p, key, structures);
    p << "\n";
  }

  this->all(p, keys);

  footer(p, snapshot.namespaces);
}

void CPPJsonCodeGenerator::header(Printer & p, const ir::Namespaces & n,
    const bool k) {
  std::string ns;

  for (const auto & item : n) {
    if ( ! ns.empty()) {
      ns += "::";
    }
    ns += item;
  }

  p << "#include <algorithm>" << "\n"
    << "#include <cstdlib>" << "\n"
    << "#include <cstring>" << "\n"
    << "#include <sstream>" << "\n"
    << "\n"
    << "#include \"configuration-json.h\"" << "\n"
    << "\n"
    //TODO(dmorilha): this code to be reviewed
    << "extern \"C\" {" << "\n"
    << "void json(const char * * v, const int a, const char * * k," << "\n"
    << tab(2) << "const int b, char * * o, int * s) {" << "\n"
    << tab(1) << "using namespace " << ns << ";" << "\n"
    << tab(1) << "Context context = Context::Create(v, a);" << "\n"
    << tab(1) << "ConfigurationJson json(context);" << "\n"
    << tab(1) << "std::string c;" << "\n"
    << tab(1) << "if (k != NULL || b > 0) {" << "\n"
    << tab(2) << "if (*k[0] == '*') {" << "\n"
    << tab(3) << "json.all(c);" << "\n"
    << tab(2) << "} else {" << "\n"
    << tab(3) << "json.keys(k, b, c);" << "\n"
    << tab(2) << "}" << "\n"
    << tab(1) << "} else {" << "\n";

  if (k) {
    p << tab(2) << "Configuration configuration(context);" << "\n"
      << tab(2) << "typedef std::vector< const char * > Keys;" << "\n"
      << tab(2) << "Keys keys = configuration.keys();" << "\n"
      << tab(2) << "json.keys(keys.data(), keys.size(), c);" << "\n";
  } else {
    p << tab(2) << "json.all(c);" << "\n";
  }

  p << tab(1) << "}" << "\n"
    << tab(1) << "*o = static_cast< char * >(malloc(c.size() + 1));" << "\n"
    << tab(1) << "std::copy(c.data(), c.data() + c.size(), *o);" << "\n"
    << tab(1) << "*s = c.size();" << "\n"
    << tab(1) << "(*o)[*s] = '\\0';" << "\n"
    << "}" << "\n"
    << "\n"
    << "#ifndef VERSION" << "\n"
    << "#define VERSION 0" << "\n"
    << "#endif" << "\n"
    << "\n"
    << "int version(void) {" << "\n"
    << tab(2) << "return VERSION;" << "\n"
    << "}" << "\n"
    << "} //end of extern" << "\n"
    << "\n";

  if ( ! n.empty()) {
    for (const auto & item : n) {
      p << "namespace " << item << " {" << "\n";
    }
    p << "\n";
  }
}

void CPPJsonCodeGenerator::footer(Printer & p, const ir::Namespaces & n) {
  for (const auto & item : n) {
    p << "} //end of " << item << " namespace" << "\n";
  }
}

std::string CPPJsonCodeGenerator::type(const std::string & t, const ir::Kind k) const {
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
