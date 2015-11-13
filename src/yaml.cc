/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <iostream>
#include "yaml.h"

const std::string yaml::tag::Boolean = "tag:yaml.org,2002:bool";
const std::string yaml::tag::Dynamic = "tag:yaml.org,2002:dynamic";
const std::string yaml::tag::Float = "tag:yaml.org,2002:float";
const std::string yaml::tag::Integer = "tag:yaml.org,2002:int";
const std::string yaml::tag::Map = "tag:yaml.org,2002:map";
const std::string yaml::tag::Null = "tag:yaml.org,2002:null";
const std::string yaml::tag::Sequence = "tag:yaml.org,2002:seq";
const std::string yaml::tag::String = "tag:yaml.org,2002:str";

bool yaml::validate(const YAML::Node & node) {
  std::string tag = node.Tag();
  std::transform(std::begin(tag), std::end(tag), std::begin(tag), ::tolower);

  if (tag == yaml::tag::Boolean) {
    bool out;
    return YAML::convert< bool >::decode(node, out);

  } else if (tag == yaml::tag::Float) {
    double out;
    return YAML::convert< double >::decode(node, out);

  } else if (tag == yaml::tag::Integer) {
    long out;
    return YAML::convert< long >::decode(node, out);

  } else if (tag == yaml::tag::Null) {
    YAML::_Null out;
    return YAML::convert< YAML::_Null >::decode(node, out);

  } else if (tag == yaml::tag::String) {
    std::string out;
    return YAML::convert< std::string >::decode(node, out);
  }

  return true;
}

std::string yaml::infer(const YAML::Node & node) {
  switch (node.Type()) {
  case YAML::NodeType::Null:
    return yaml::tag::Null;

  case YAML::NodeType::Map:
    return yaml::tag::Map;

  case YAML::NodeType::Sequence:
    return yaml::tag::Sequence;

  case YAML::NodeType::Scalar:
    {
      long out;
      if (YAML::convert< long >::decode(node, out)) {
        return yaml::tag::Integer;
      }
    }

    {
      double out;
      if (YAML::convert< double >::decode(node, out)) {
        return yaml::tag::Float;
      }
    }

    {
      bool out;
      if (YAML::convert< bool >::decode(node, out)) {
        return yaml::tag::Boolean;
      }
    }

    {
      std::string out;
      if (YAML::convert< std::string >::decode(node, out)) {
        return yaml::tag::String;
      }
    }

  default:
    break;
  }

  return std::string();
}

std::string yaml::tag(const YAML::Node & node) {
  std::string tag = node.Tag();
  if ( ! tag.empty() && tag != "?"
      && tag != "!") {
    return tag;
  }
  return yaml::infer(node);
}
