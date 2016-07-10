/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#ifndef LIBRARY_H
#define LIBRARY_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>

namespace library {
namespace pointer {
typedef void (*json) (const char * *, const int,
    const char * *, const int, char * *, int *);
typedef int (*version) (void);
} //end of pointer namespace

class Instance {
  typedef void * Handle;

  struct Symbols {
    pointer::json json;
    pointer::version version;

    Symbols(const pointer::json j = NULL, const pointer::version v = NULL) :
      json(j), version(v) { }
  };

  Handle handle;
  Symbols symbols;

  Instance(const Handle h, const pointer::json j, const pointer::version v) :
    handle(h), symbols(j, v) { }

public:
  ~Instance();

  void json(const char * *, const int, const char * *,
      const int, char * *, int *) const;
  int version(void) const;

  friend class Library;
};

struct Library {
  typedef boost::shared_ptr< Instance > Pointer;
  typedef std::pair< std::string, Pointer > Pair;

private:
  const std::string file_;
  const int n_;
  Pair * const instances_;
  int x_;

public:
  ~Library();
  Library(const char * const, const int n = 1);

  Pointer get(void) const;
  Pointer get(const std::string & v) const;
  bool reload(void);
  const char * file(void) const;
};
} //end of library namespace

#endif //LIBRARY_H
