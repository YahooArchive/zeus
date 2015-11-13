/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef YAML_H
#define YAML_H

#include <string>

#include <yaml-cpp/node/convert.h>
#include <yaml-cpp/yaml.h>

struct yaml {
  struct tag {
    static const std::string Boolean;
    static const std::string Dynamic;
    static const std::string Float;
    static const std::string Integer;
    static const std::string Map;
    static const std::string Null;
    static const std::string Sequence;
    static const std::string String;
  };

  static bool validate(const YAML::Node &);
  static std::string infer(const YAML::Node &);
  static std::string tag(const YAML::Node &);
};

#endif
