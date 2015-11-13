/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <regex>
#include <set>
#include <string>

typedef std::shared_ptr< const std::regex > Regex;
typedef std::shared_ptr< const std::set< std::string > > Set;

#endif //COMMON_H
