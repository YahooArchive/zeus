/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>
#include <string>
#include <ts/apidefs.h>
#include <ts/ts.h>
#include <vector>

namespace context {
typedef std::map< std::string, std::string > Context;
typedef std::vector< const char * > Parameters;

void toJson(const Parameters &, std::string &);
void toQueryString(const Parameters &, const int, std::string &);
void extractBuckets(const std::string &, Context &);
std::string & numberToString(const int, std::string &);
void fromQueryString(const std::string &, const std::string &, Context &);
void fromQueryParameter(const std::string &, Context &);
void percentEncode(const char * const, const int, std::string &);
} //end of context namespace
#endif //CONTEXT_H
