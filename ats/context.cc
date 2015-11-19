/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>
#include <cstdio>
#include <cstring>

#include "common.h"
#include "context.h"

void context::toJson(const Parameters & p, std::string & o) {
  const int size = p.size();
  bool first = true;
  for (int i = 0; i < size; i += 2) {
    if (p[i] != NULL) {
      assert(p[i + 1] != NULL);
      if (first) {
        first = false;
      } else {
        o += ",";
      }
      o += "\"";
      o += p[i];
      o += "\":\"";
      o += p[i + 1];
      o += "\"";
    }
  }
}

void context::percentEncode(const char * const c, const int l, std::string & o) {
  const size_t size = l * 5;
  char buffer[size];
  size_t length = 0;
  CHECK(TSStringPercentEncode(c, l, buffer, size, &length, NULL));
  if (length > 0) {
    o += std::string(buffer, length);
  } else {
    o += std::string(c, l);
  }
}

void context::toQueryString(const Parameters & p, const int v, std::string & o) {
  const int size = p.size();
  bool first = true;

  for (int i = 0; i < size; i += 2) {
    if (p[i] != NULL) {
      assert(p[i + 1] != NULL);
      if (first) {
        first = false;
      } else {
        o += "&";
      }
      percentEncode(p[i], strlen(p[i]), o);
      o += "=";
      percentEncode(p[i + 1], strlen(p[i + 1]), o);
    }
  }

  if (v > 0 ) {
    if (first) {
      first = false;
    } else {
      o += "&";
    }
    o += "version=";
    {
      std::string buffer;
      o += numberToString(v, buffer);
    }
  }
}

void context::extractBuckets(const std::string & v, Context & c) {
  fromQueryString(v, "layer_", c);
}


std::string & context::numberToString(const int v, std::string & o) {
  //TODO(dmorilha): not sure how big buffer has to be.
  char buffer[128];
  if (sprintf(buffer, "%d", v) > 0) {
    o = buffer;
  }
  return o;
}

template< char A, char B, class C >
void fromString(const std::string & v, const std::string & p, C & c) {
  const char * const begin = v.data(),
        * const end = begin + v.size(),
        * a = begin, * b = NULL,
        * iterator = begin;

  for (; iterator != end; ++iterator) {
    assert(*iterator != '\0');
    switch (*iterator) {
      case A:
        b = iterator + 1;
        break;
      case B:
        if (b != NULL && b - a > 1) {
          if (iterator - b > 0) {
            std::string dimension = p;
            dimension += std::string(a, b - 1);
            c[dimension] = std::string(b, iterator);
          }
          b = NULL;
        }
        a = iterator + 1;
        break;

      default:
        break; //skip
    }
  }

  if (b != NULL && b - a > 1 && iterator - b > 0) {
    std::string dimension = p;
    dimension += std::string(a, b - 1);
    c[dimension] = std::string(b, iterator);
  }
}

void context::fromQueryString(const std::string & v, const std::string & p, Context & c) {
  return fromString< '=', '&' >(v, p, c);
}

void context::fromQueryParameter(const std::string & v, Context & c) {
  return fromString< ':', ',' >(v, "", c);
}
