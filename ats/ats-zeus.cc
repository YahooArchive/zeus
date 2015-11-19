/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <sstream>
#include <string>
#include <ts/apidefs.h>
#include <ts/remap.h>
#include <ts/ts.h>
#include <unistd.h>
#include <vector>

#include "common.h"
#include "library.h"

#ifndef PLUGIN_TAG
#error Please define a PLUGIN_TAG before including this file.
#endif

static const char * const VERSION = "version";
static const char * const KEYS = "keys";

static int maxAge = 0;

static struct {
  int hits;
  int notFounds;
  int size; //average
  int time; //average
  int updates;
} statistics;

void initializeStatistics(void) {
  statistics.hits = TSStatCreate(PLUGIN_TAG "_service" ".hits", TS_RECORDDATATYPE_INT,
      TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_COUNT);

  statistics.notFounds = TSStatCreate(PLUGIN_TAG "_service" ".notFounds", TS_RECORDDATATYPE_INT,
      TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_COUNT);

  statistics.size = TSStatCreate(PLUGIN_TAG "_service" ".size", TS_RECORDDATATYPE_INT,
      TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_AVG);

  statistics.time = TSStatCreate(PLUGIN_TAG "_service" ".time", TS_RECORDDATATYPE_INT,
      TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_AVG);

  statistics.updates = TSStatCreate(PLUGIN_TAG "_service" ".updates", TS_RECORDDATATYPE_INT,
      TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_COUNT);
}

TSReturnCode TSRemapInit(TSRemapInterface *, char *, int) {
  TSDebug(PLUGIN_TAG, "init");

  TSPluginRegistrationInfo info;

  info.plugin_name = const_cast< char * >(PLUGIN_TAG);
  info.support_email = const_cast< char * >("search-l7-dev@yahoo-inc.com");
  info.vendor_name = const_cast< char * >("Yahoo Inc");

  if (TSPluginRegister(TS_SDK_VERSION_3_0, &info) != TS_SUCCESS) {
    TSError("[" PLUGIN_TAG "] plugin registration failed.\n");
  }

  {
    const char * const maxAgeEnv = getenv(PLUGIN_TAG "__max_age");

    if (maxAgeEnv != NULL) {
      maxAge = atoi(maxAgeEnv);
      if (maxAge < 0 || maxAge > 31536000) {
        TSError("[" PLUGIN_TAG "] Cache-Control max-age is out of range (0..31536000): %i."
            " Resetting to 0 (disabled).", maxAge);
        maxAge = 0;
      } else {
        TSDebug(PLUGIN_TAG, "Cache-Control max-age set to %d.", maxAge);
      }
    }
  }

  initializeStatistics();

  return TS_SUCCESS;
}

int ServerUpdate(TSCont c, TSEvent e, void *) {
  using namespace library;
  assert(c != NULL);
  assert(e == TS_EVENT_MGMT_UPDATE);
  Library * const library = static_cast< Library * >(TSContDataGet(c));
  assert(library != NULL);
  library->reload();
  TSStatIntIncrement(statistics.updates, 1);
  TSDebug(PLUGIN_TAG, "Updating the server config.");
  return 0;
}

TSReturnCode TSRemapNewInstance(int c, char * * v, void * * i, char *, int) {
  using namespace library;

  TSDebug(PLUGIN_TAG, "new instance");
  assert(c >= 3);
  TSDebug(PLUGIN_TAG, "config library: %s", v[2]);
  Library * const library = new Library(v[2], 5);
  assert(library != NULL);
  *i = library;
  const TSCont continuation = TSContCreate(ServerUpdate, NULL);
  assert(continuation != NULL);
  TSContDataSet(continuation, library);
  TSMgmtUpdateRegister(continuation, PLUGIN_TAG);
  return TS_SUCCESS;
}

void TSRemapDeleteInstance(void * i) {
  assert(i != NULL);
  TSDebug(PLUGIN_TAG, "delete");
  delete static_cast< library::Library * >(i);
}

struct Data {
  char * data;
  int size;
  std::string context;
  std::string version;
  int cache;
  struct timespec start;

  ~Data() {
    if (data != NULL) {
      assert(size > 0);
      free(data);
    }
  }

  Data(void) : data(NULL), size(0), cache(0) {
    clock_gettime(CLOCK_MONOTONIC, &start);
  }
};

int ServerIntercept(TSCont c, TSEvent e, void * d) {
  assert(c != NULL);
  Data * const data = static_cast< Data * >(TSContDataGet(c));

  assert(e == TS_EVENT_ERROR
      || e == TS_EVENT_NET_ACCEPT
      || e == TS_EVENT_NET_ACCEPT_FAILED
      || e == TS_EVENT_VCONN_WRITE_COMPLETE);

  TSDebug(PLUGIN_TAG, "event: %i", e);

  if (e == TS_EVENT_NET_ACCEPT) {
    assert(d != NULL);
    assert(data != NULL);
    const TSVConn vconnection = static_cast< TSVConn >(d);

    TSIOBuffer buffer = TSIOBufferCreate();
    assert(buffer != NULL);
    TSIOBufferReader reader = TSIOBufferReaderAlloc(buffer);
    assert(reader != NULL);

    if (data->version.empty()) {
      data->version = "0";
    }

    if (data->size > 0 || data->context.size() > 0) {
      const int size = data->size + data->context.size() + data->version.size() + 33;

      std::stringstream header;

      header << "HTTP/1.1 200 OK" "\r\n"
        "Content-Type: application/javascript; charset=UTF-8" "\r\n"
        "Content-Length: " << size << "\r\n";

      if (data->cache > 0) {
        header << "Cache-Control: max-age=" << data->cache << "\r\n";
        TSDebug(PLUGIN_TAG, "setting Cache-Control max-age to %d seconds.", data->cache);
      }

      header << "\r\n";

      {
        const std::string h = header.str();
        TSIOBufferWrite(buffer, h.data(), h.size());
      }

      TSIOBufferWrite(buffer, "{\"context\":{", 12);
      TSIOBufferWrite(buffer, data->context.data(), data->context.size());
      TSIOBufferWrite(buffer, "},\"data\":", 9);
      assert(strlen(data->data) == data->size);
      TSIOBufferWrite(buffer, data->data, data->size);
      TSIOBufferWrite(buffer, ",\"version\":", 11);
      TSIOBufferWrite(buffer, data->version.data(), data->version.size());
      TSIOBufferWrite(buffer, "}", 1);

      TSStatIntIncrement(statistics.hits, 1);
      TSStatIntIncrement(statistics.size, size);

    } else {
      const std::string response = "HTTP/1.1 404 NOT FOUND" "\r\n"
        "Content-Type: text/html; charset=UTF-8" "\r\n"
        "Content-Length: 18" "\r\n"
        "\r\n"
        "<h2>NOT FOUND</h2>";

      TSIOBufferWrite(buffer, response.data(), response.size());
      TSStatIntIncrement(statistics.notFounds, 1);
    }

    const TSVIO vio = TSVConnWrite(vconnection, c,
        reader, TSIOBufferReaderAvail(reader));

    assert(vio != NULL);

  } else {
    if (e == TS_EVENT_VCONN_WRITE_COMPLETE) {
      assert(data != NULL);

      struct timespec end;
      clock_gettime(CLOCK_MONOTONIC, &end);

      const long diff = (end.tv_sec - data->start.tv_sec) * 1000000
        + (end.tv_nsec - data->start.tv_nsec) / 1000;

      if (data->size > 0 || data->context.size() > 0) {
        TSStatIntIncrement(statistics.time, diff);
      }

      TSDebug(PLUGIN_TAG, "Service took %li microseconds", diff);
    }

    if (data != NULL) {
      delete data;
      TSContDataSet(c, NULL);
    }

    TSContDestroy(c);
  }

  return 0;
}

TSRemapStatus TSRemapDoRemap(void * i, TSHttpTxn t, TSRemapRequestInfo *) {
  using namespace library;
  typedef std::vector< const char * > Strings;

  TSDebug(PLUGIN_TAG, "remap");

  Library * library = static_cast< Library * >(i);
  assert(library != NULL);
  assert(t != NULL);

  Data * data = new Data();
  assert(data != NULL);

  char * const query = NULL;
  Strings parameters, keys;
  const char * version = NULL;

  TSMBuffer buffer;
  TSMLoc header;

  CHECK(TSHttpTxnClientReqGet(t, &buffer, &header));
  assert(buffer != NULL);
  assert(header != NULL);

  TSMLoc url;

  CHECK(TSHttpHdrUrlGet(buffer, header, &url));
  assert(url != NULL);

  int length;
  const char * const pointer = TSUrlHttpQueryGet(buffer, url, &length);

  if (pointer != NULL) {
    assert(length > 0);
    char * const query = new char [length + 1];
    size_t length2;
    CHECK(TSStringPercentDecode(pointer, length, query, length, &length2));
    assert(length2 > 0);
    assert(length2 <= length);
    query[length2] = '\0';

    char * c = query;

    const char * a = c,
          * b = NULL;

    for (; c < query + length2; ++c) {
      if (*c == '=') {
        *c = '\0';
        b = c + 1;
        if (strcmp(KEYS, a) == 0) {
          a = KEYS;
        } else if (strcmp(VERSION, a) == 0) {
          a = VERSION;
        }
      } else if (*c == ',' && KEYS == a) {
        *c = '\0';
        keys.push_back(b);
        b = c + 1;
      } else if (*c == '&') {
        *c = '\0';
        if (b != NULL) {
          if (c - b > 0) {
            TSDebug(PLUGIN_TAG, "query parameter: %s = %s", a, b);
            if (VERSION == a) {
              version = b;
            } else if (KEYS == a) {
              keys.push_back(b);
            } else if (b - a > 1) {
              parameters.push_back(a);
              parameters.push_back(b);
            }
          }
          b = NULL;
        }
        a = c + 1;
      }
    }

    if (b != NULL && c - b > 0) {
      TSDebug(PLUGIN_TAG, "query parameter: %s = %s", a, b);
      if (VERSION == a) {
        version = b;
      } else if (KEYS == a) {
        keys.push_back(b);
      } else if (b - a > 1) {
        parameters.push_back(a);
        parameters.push_back(b);
      }
    }
  }

  Library::Pointer instance;

  {
    bool hasVersion = version != NULL;

    TSHttpTxnRespCacheableSet(t, hasVersion);

    if (hasVersion) {
      TSDebug(PLUGIN_TAG, "Version is %s", version);
      instance = library->get(version);
      data->cache = maxAge;
    } else {
      instance = library->get();
    }
  }

  if (instance) {
    const size_t size = parameters.size();
    TSDebug(PLUGIN_TAG, "library for look-up: %s", library->file());

    if (keys.empty()) {
      instance->json(parameters.data(), size, NULL,
          0, &(data->data), &(data->size));
    } else {
      if (unlikely(TSIsDebugTagSet(PLUGIN_TAG) > 0)) {
        const Strings::const_iterator end = keys.end();
        Strings::const_iterator iterator = keys.begin();

        std::string output;

        for (; iterator != end; ++iterator) {
          output += "\n" " - ";
          output += *iterator;
        }

        TSDebug(PLUGIN_TAG, "keys are:%s", output.c_str());
      }

      instance->json(parameters.data(), size, keys.data(),
          keys.size(), &(data->data), &(data->size));
    }

    {
      const int version = instance->version();
      if (version > 0) {
        char buffer[32];
        snprintf(buffer, 32, "%d", version);
        data->version = buffer;
      }
    }

    for (int i = 0, j = 0; i < size; i += 2) {
      if (parameters[i] != NULL) {
        assert(parameters[i + 1] != NULL);
        if (j > 0) {
          data->context += ",";
        }
        data->context += "\"";
        data->context += parameters[i];
        data->context += "\":\"";
        data->context += parameters[i + 1];
        data->context += "\"";
        ++j;
      }
    }
  }

  if (query != NULL) {
    delete [] query;
  }

  const TSCont continuation = TSContCreate(ServerIntercept, TSMutexCreate());
  assert(continuation != NULL);
  TSContDataSet(continuation, data);
  TSHttpTxnServerIntercept(continuation, t);

  return TSREMAP_NO_REMAP_STOP;
}
