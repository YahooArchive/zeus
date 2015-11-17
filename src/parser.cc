/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>
#include <iostream>
#include <string>
#include <utility>

#include "key.h"
#include "parser.h"

namespace parser {
Type::TYPES Parser::TagToType(const std::string & tag) {
  Type::TYPES result = Type::kUndefined;

  if (tag == yaml::tag::Sequence) {
    result = Type::kArray;
  } else if (tag == yaml::tag::Map) {
    result = Type::kObject;
  } else if (tag == yaml::tag::Dynamic) {
    result =  Type::kDynamic;
  } else if (tag == yaml::tag::Boolean) {
    result =  Type::kBoolean;
  } else if (tag == yaml::tag::Float) {
    result =  Type::kFloat;
  } else if (tag == yaml::tag::Integer) {
    result =  Type::kInteger;
  } else if (tag == yaml::tag::String) {
    result =  Type::kString;
  } else {
    result =  Type::kUnknown;
  }

  return result;
}

std::pair< std::string, bool >
  Parser::getString(const YAML::Node & node) const {

  std::string key;

  if ( ! node.IsScalar()) {
    //std::cerr << "key is not scalar, ignoring it" << std::endl;
    return std::make_pair(key, false);
  }

  if ( ! (YAML::convert< std::string >::decode(node, key)
        || key.empty())) {
    //std::cerr << "invalid key, ignoring it" << std::endl;
    return std::make_pair(key, false);
  }

  return std::make_pair(key, true);
}

//TODO(dmorilha): YCB supports sequences here, not sure why
Contexts Parser::processSettings(Parser::Result & r,
    const YAML::Node & n) const {
  using namespace dimensions;

  if (n.IsScalar()) {
    const auto result = getString(n);
    if (std::get< 1 >(result)) {
      const auto value = std::get< 0 >(result);
      assert(value == "master");
    }
    return { Context() };
  }

  assert(n.IsMap());

  std::vector< DimensionTable::Input > input;
  input.emplace_back();

  for (const auto & node : n) {
    const auto r1 = getString(node.first);
    if (std::get< 1 >(r1)) {
      if (node.second.IsScalar()) {
        const auto r2 = getString(node.second);
        assert(std::get< 1 >(r2));
        for (auto & item : input) {
          item.emplace_back(std::get< 0 >(r1), std::get< 0 >(r2));
        }
      } else if (node.second.IsSequence()) {
        const auto size = node.second.size(),
              size2 = input.size();
        assert(size2 > 0);
        //just size - 1 times.
        for (size_t i = 0; i < size - 1; ++i) {
          for (size_t j = 0; j < size2; ++j) {
            input.push_back(input[j]);
          }
        }
        assert(size == 0 || input.size() == size * size2);
        size_t i = 0;
        for (const auto & item : node.second) {
          const auto r2 = getString(item);
          assert(std::get< 1 >(r2));
          for (size_t j = 0; j < size2; ++j) {
            assert(i % size2 == j);
            input[i].emplace_back(std::get< 0 >(r1), std::get< 0 >(r2));
            ++i;
          }
        }
      } else {
        assert(false); //unrecheable;
      }
    }
  }

  Contexts result;
  for (const auto & item : input) {
    result.emplace_back(r.dimensions.context(item));
  }
  return result;
}

void Parser::processMap(const YAML::Node & n, const Result & r, Value & v) const {
  assert(v.type == Type::kObject
      || v.type == Type::kDynamic);
  assert(n.IsMap());
  for (const auto & item : n) {
    const auto result = getString(item.first);
    if (std::get< 1 >(result)) {
      const std::string key = std::get< 0 >(result);
      auto & property = v.push();
      property.first = key;
      processValue(item.second, r, property.second);
    }
  }
}

void Parser::processSequence(const YAML::Node & n, const Result & r, Value & v) const {
  assert(n.IsSequence());
  for (const auto & item : n) {
    auto & property = v.push();
    processValue(item, r, property.second);
  }
}

void Parser::processValue(const YAML::Node & n, const Result & r, Value & v) const {
  const auto tag = yaml::tag(n);
  const Type::TYPES typeFromTag = TagToType(tag);
  std::string content;

  YAML::convert< std::string >::decode(n, content);

  switch (typeFromTag) {
  case Type::kArray:
    v.type = typeFromTag;
    processSequence(n, r, v);
    break;

  case Type::kObject:
  case Type::kDynamic:
    v.type = typeFromTag;
    processMap(n, r, v);
    break;

  case Type::kBoolean:
    {
      v.type = typeFromTag;
      bool val = false;
      YAML::convert< bool >::decode(n, val);
      content = val ? "true" : "false";
    }
    break;

  case Type::kUnknown:
    {
      const Type::TYPES inferredType = TagToType(yaml::infer(n));
      const auto tag1 = tag.substr(1);
      bool found = false;

      /*
       * for objects, tags are used as type aliases
       */
      if (inferredType == Type::kObject) {
        v.alias = tag1;
        found = true;
      }

      /*
       * TODO(dmorilha): handle collisions. What happens when
       * the same key is used for regular expression and set?
       */
      if ( ! found) {
        const RegularExpressions::const_iterator item = r.regexs.find(tag1);
        if ((found = item != std::end(r.regexs))) {
          assert(static_cast< bool >(item->second));
          v.regex = item->second;
          if ( ! std::regex_match(content, *item->second)) {
            std::cerr << "content \"" << content << "\" did not match regular expression \"" << tag1 << "\"" << std::endl;
            assert(false); //invalid content
          }
        }
      }

      if ( ! found) {
        const Sets::const_iterator item = r.sets.find(tag1);
        if ((found = item != std::end(r.sets))) {
          assert(static_cast< bool >(item->second));
          v.set_ = item->second;
          if (item->second->find(content) == std::end(*item->second)) {
            std::cerr << "content \"" << content << "\" did not match set \"" << tag1 << "\"" << std::endl;
            assert(false); //invalid content
          }
        }
      }

      if (found) {
        v.type = inferredType;

        switch (inferredType) {
        case Type::kArray:
          processSequence(n, r, v);
          break;

        case Type::kObject:
        case Type::kDynamic:
          processMap(n, r, v);
          break;

        default:
          break;
        }

      } else {
        std::cerr << "unsuported tag: " << tag << std::endl;
        assert(false); //unrecognized type
      }
    }
    break;

  default:
    v.type = typeFromTag;
    break;
  }

  v.content = content;
}

void Parser::processKeyKeys(Parser::Result & r, const Contexts & c,
    const YAML::Node & n) const {
  Value value;

  if ( ! n.IsSequence()) {
    std::cerr << "Special key \"keys\" has to be a sequence of strings." << std::endl;
    assert(false);
  }

  value.type = Type::kArray;

  for (const auto & item : n) {
    const auto tag = yaml::tag(item);
    const Type::TYPES type = TagToType(tag);

    if (type != Type::kString) {
      std::cerr << "Special key \"keys\" has to be a sequence of strings." << std::endl;
      assert(false);
    }

    auto & property = value.push();
    YAML::convert< std::string >::decode(item, property.second.content);
    property.second.type = type;
  }

  for (const auto & item : c) {
    r.keys["keys"].key.add(item, value);
  }
}

void Parser::processKey(Parser::Result & r, const Contexts & c,
    const std::string & k, const YAML::Node & n) const {
  Value value;
  processValue(n, r, value);
  for (const auto & item : c) {
    r.keys[k].key.add(item, value);
  }
}

void Parser::processContext(Parser::Result & r, const YAML::Node & n,
    const YAML::Node & o) const {
  assert(n.IsMap());
  const auto contexts = processSettings(r, o);
  for (const auto & item : n) {
    const auto result = getString(item.first);
    if (std::get< 1 >(result)) {
      const std::string key = std::get< 0 >(result);
      if (key == "settings") { continue; }
      if (key == "keys") {
        processKeyKeys(r, contexts, item.second);
      } else {
        processKey(r, contexts, key, item.second);
      }
    }
  }
}

//TODO(dmorilha): YCB supports sequences of maps with multiple levels.
void Parser::processDimensions(Result & r, const YAML::Node & n) const {
  assert(n.IsSequence());
  for (const auto & item : n) {
    const auto result = getString(item);
    if (std::get< 1 >(result)) {
      const std::string key = std::get< 0 >(result);
      r.dimensions.insert(key);
    }
  }
}

void Parser::processNamespaces(Result & r, const YAML::Node & n) const {
  assert(n.IsSequence());
  for (const auto & item : n) {
    const auto result = getString(item);
    if (std::get< 1 >(result)) {
      const std::string key = std::get< 0 >(result);
      r.namespaces.push_back(key);
    }
  }
}

void Parser::processRegularExpressions(Result & r, const YAML::Node & n) const {
  assert(n.IsMap());
  for (const auto & node : n) {
    const auto r1 = getString(node.first);
    if (std::get< 1 >(r1)) {
      assert(node.second.IsScalar());
      const auto r2 = getString(node.second);
      if (std::get< 1 >(r2)) {
        const std::string key = std::get< 0 >(r1),
          value = std::get< 0 >(r2);
        try {
          r.regexs[key].reset(new std::regex(value,
              std::regex_constants::ECMAScript | std::regex_constants::nosubs));
        } catch (std::exception & e) {
          std::cerr << "error while processing regular expression \"" << value << "\"" << std::endl;
        }
      }
    }
  }
}

void Parser::processSets(Result & r, const YAML::Node & n) const {
  assert(n.IsMap());
  for (const auto & node : n) {
    const auto r1 = getString(node.first);
    if (std::get< 1 >(r1)) {
      assert(node.second.IsSequence());
      const std::string key = std::get< 0 >(r1);
      Set & set = r.sets[key];

      if ( ! static_cast< bool >(set)) {
        set.reset(new std::set< std::string >);
      }

      for (const auto & item : node.second) {
        assert(item.IsScalar());
        const auto r2 = getString(item);
        if (std::get< 1 >(r2)) {
          set->insert(std::get< 0 >(r2));
        }
      }
    }
  }
}

void Parser::parse(const YAML::Node & root, Result & r) const {
  assert(root.IsSequence() || root.size() == 0);
  for (const auto & item : root) {
    assert(item.IsMap());
    //first pass to find dimensions, namespaces and regular-expressions
    for (const auto & node : item) {
      const auto result = getString(node.first);
      if (std::get< 1 >(result)) {
        const std::string key = std::get< 0 >(result);
        if (key == "dimensions") {
          processDimensions(r, node.second);
        } else if (key == "namespaces") {
          processNamespaces(r, node.second);
        } else if (key == "regular-expressions") {
          processRegularExpressions(r, node.second);
        } else if (key == "sets") {
          processSets(r, node.second);
        }
      }
    }

    //second pass to find everything else
    for (const auto & node : item) {
      const auto result = getString(node.first);
      if (std::get< 1 >(result)) {
        const std::string key = std::get< 0 >(result);
        if (key == "settings") {
          processContext(r, item, node.second);
          break;
        }
      }
    }
  }
}
} //end of parser namespace
