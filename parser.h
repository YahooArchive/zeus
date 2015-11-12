/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common.h"
#include "context.h"
#include "dimension.h"
#include "key.h"
#include "source-map.h"
#include "yaml.h"

struct Key;

namespace parser {

struct Parser {
  typedef std::vector< std::string > Namespaces;
  typedef std::map< std::string, Regex > RegularExpressions;
  typedef std::shared_ptr< std::set< std::string > > Set;
  typedef std::map< std::string, Set > Sets;

  struct Result {
    dimensions::DimensionTable dimensions;
    KeyTable keys;
    struct {
      ContextSourceMap context;
      ValueSourceMap value;
    } sourceMap;
    Namespaces namespaces;
    RegularExpressions regexs;
    Sets sets;
  };

  static Type::TYPES TagToType(const std::string &);

  void parse(const YAML::Node &, Result &) const;

  std::pair< std::string, bool > getString(const YAML::Node &) const;

  void processContext(Result &, const YAML::Node &,
      const YAML::Node &) const;

  Contexts processSettings(Result &, const YAML::Node &) const;

  void processDimensions(Result &, const YAML::Node &) const;

  void processKey(Result &, const Contexts &, const std::string &,
      const YAML::Node &) const;

  void processKeyKeys(Result &, const Contexts &, const YAML::Node &) const;

  void processMap(const YAML::Node &, const Result &, Value &) const;

  void processSequence(const YAML::Node &, const Result &, Value &) const;

  void processValue(const YAML::Node &, const Result &, Value &) const;

  void processNamespaces(Result &, const YAML::Node &) const;

  void processRegularExpressions(Result &, const YAML::Node &) const;

  void processSets(Result &, const YAML::Node &) const;
};

} //end of parser namespace

#endif
