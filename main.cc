/*
 * Copyright (c) 2015, Yahoo Inc. All rights reserved.
 * Copyrights licensed under the New BSD License.
 * See the accompanying LICENSE file for terms.
 */

#include <iostream>
#include <sstream>

#include <assert.h>

#include <yaml-cpp/yaml.h>

#include "graph-builder.h"
#include "graph-printer.h"
#include "graph-trimmer.h"
#include "graph-type-extractor.h"
#include "graph-type-propagator.h"
#include "graph.h"
#include "ir.h"
#include "key.h"
#include "parser.h"
#include "structure-writer.h"
#include "structure.h"
#include "yaml.h"

//generators
#include "cpp-code.h"
#include "cpp-header.h"
#include "cpp-json-code.h"
#include "cpp-json-header.h"
#include "java.h"
#include "js.h"
#include "php.h"

bool handleSpecialKeys(KeyTable::Entries::value_type & k) {
  bool result = false;

  if (k.first == "keys") {
    k.second.key.type = StructureTable::kString;
    k.second.key.kind = ir::Kind::kArray;
    result = true;
  }

  return result;
}

int main(int argc, char * * argv) {

  bool cppCode = false,
    cppHeader = false,
    cppJsonCode = false,
    cppJsonHeader = false,
    java = false,
    js = false,
    php = true;

  std::vector< const char * > files;

  ir::Snapshot snapshot;

  //TODO(dmorilha): stays here until we extract the types.
  StructureTable structures;

  for (int i = 1; i < argc; ++i) {
    if (*(argv[i]) == '-') {
      cppCode |= strcmp(argv[i] + 1, "-cpp-code") == 0;
      cppHeader |= strcmp(argv[i] + 1, "-cpp-header") == 0;
      cppJsonCode |= strcmp(argv[i] + 1, "-cpp-json-code") == 0;
      cppJsonHeader |= strcmp(argv[i] + 1, "-cpp-json-header") == 0;
      java |= strcmp(argv[i] + 1, "-java") == 0;
      js |= strcmp(argv[i] + 1, "-js") == 0;
      php |= strcmp(argv[i] + 1, "-php") == 0;

      if (strcmp(argv[i] + 1, "-set") == 0) {
        ++i;
        assert(i < argc); //--set requires two arguments
      }

    } else {
      files.push_back(argv[i]);
    }
  }

  try {
    using namespace parser;

    Parser::Result r;

    for (const auto & file : files) {
      Parser parser;
      const auto root = YAML::LoadFile(file);
      parser.parse(root, r);
    }

    snapshot.namespaces = std::move(r.namespaces);

    GraphBuilder builder;

    GraphTrimmer trimmer = GraphTrimmer::Create(r.dimensions, argv, argc);
    trimmer.markSkip(r.dimensions);

    //TODO(dmorilha) we can try this process in parallel...
    for (auto & item : r.keys.entries) {
      Key & key = item.second.key;
      Graph & graph = key.graph;
      contextSort(graph);

      trimmer.trim(key.graph);

      if ( ! handleSpecialKeys(item)) {
        GraphTypeExtractor typeExtractor;
        typeExtractor.extract(key, structures);

        GraphTypePropagator propagator;
        propagator.propagate(key);
      }

      snapshot.keys.emplace_back(
        builder.build(item.first, graph, r.dimensions));

      {
        ir::Key & key2 = snapshot.keys.back();
        key2.kind = key.kind;
        key2.type = structures.getTypeName(key.type);
      }
    }

    {
      StructureWriter writer;
      writer(structures, snapshot.structures);
    }

    snapshot.dimensions = r.dimensions.enumerate();
  } catch (const std::exception & e) { }

  //TODO(dmorilha): validate keys against the actual keys

  Printer p(std::cout);

  Generator::Pointer generator;

  if (cppCode) {
    generator.reset(new CPPCodeGenerator());
  } else if (cppHeader) {
    generator.reset(new CPPHeaderGenerator());
  } else if (cppJsonCode) {
    generator.reset(new CPPJsonCodeGenerator());
  } else if (cppJsonHeader) {
    generator.reset(new CPPJsonHeaderGenerator());
  } else if (java) {
    generator.reset(new JavaGenerator());
  } else if (js) {
    generator.reset(new JSGenerator());
  } else if (php) {
    generator.reset(new PHPGenerator());
  } else {
    std::cout << "Available options are" << "\n"
      << " --cpp-code: generates C++ code ouput." << "\n"
      << " --cpp-header: generates C++ header output." << "\n"
      << " --java: generates Java output." << "\n"
      << " --js: generates JS output." << "\n"
      << " --php: generates PHP output." << "\n"
      << " --set dimension:value[,value...] "
      "limits a certain dimension to only these values." << "\n";

    return 0;
  }

  assert(static_cast< bool >(generator));

  generator->generate(p, snapshot);

  return 0;
}
