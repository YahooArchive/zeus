/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>
#include <cstdio>
#include <dlfcn.h>

#include "library.h"

namespace library {

Instance::~Instance() {
  assert(handle != NULL);
  const int r = dlclose(handle);
  assert(r == 0);
  handle = NULL;
  assert(symbols.json != NULL);
  symbols.json = NULL;
  assert(symbols.version != NULL);
  symbols.version = NULL;
}

void Instance::json(const char * * a, const int b, const char * * c,
    const int d, char * * e, int * f) const {
  assert(symbols.json != NULL);
  return (*symbols.json)(a, b, c, d, e, f);
}

int Instance::version(void) const {
  assert(symbols.version != NULL);
  return (*symbols.version)();
}

Library::~Library() {
  assert(instances_ != NULL);
  delete [] instances_;
}

Library::Library(const char * const f, const int n) :
  file_(f), n_(n), instances_(new Pair[n]), x_(n - 1) {
  assert(n_ > 0);
  assert(x_ >= 0);
  assert(instances_ != NULL);
  reload();
}

Library::Pointer Library::get(void) const {
  return instances_[x_].second;
}

Library::Pointer Library::get(const std::string & s) const {
  for (int i = 0; i < n_; ++i) {
    Pair & pair = instances_[i];
    if (pair.first == s) {
      return pair.second;
    }
  }
  return Pointer();
}

bool Library::reload(void) {
  //TODO(dmorilha) only update if file content changed.
  //HACK TO BYPASS dlopen STUPID CACHE
  static int i = 0;
  std::string f;
  f.append(i, '/');
  //NO MORE THAN 100 VERSIONS
  i = (i + 1) % 100;
  f += file_;
  Instance::Handle handle = dlopen(f.c_str(), RTLD_LAZY | RTLD_LOCAL);

  x_ = (x_ + 1) % n_;

  if (handle != NULL) {
    const int SIZE = 32;
    char buffer[SIZE];
    void * const json = dlsym(handle, "json");
    void * const version = dlsym(handle, "version");
    if (json != NULL && version != NULL) {
      instances_[x_].second.reset(new Instance(handle,
          reinterpret_cast< pointer::json >(json),
          reinterpret_cast< pointer::version >(version)));

      snprintf(buffer, SIZE, "%d",
          instances_[x_].second->version());

      instances_[x_].first = buffer;

      return true;
    }
  }
  instances_[x_].second.reset();
  instances_[x_].first = "0";
  return false;
}

const char * Library::file(void) const {
  return file_.c_str();
}

} //end of library namespace
