/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef SOURCE_MAP_H
#define SOURCE_MAP_H
#include <map>

#include "context.h"
#include "value.h"
#include "yaml.h"

typedef std::map< Context *, YAML::Node > ContextSourceMap;
typedef std::map< Value *, YAML::Node > ValueSourceMap;

#endif //SOURCE_MAP_H
