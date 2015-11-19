/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef COMMON_H
#define COMMON_H

#define CHECK(X) { \
  const TSReturnCode r = static_cast< TSReturnCode >(X); \
  assert(r == TS_SUCCESS); \
}

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &); \
  void operator = (const TypeName &)

#define unlikely(x) __builtin_expect((x),0)


#endif //COMMON_H
