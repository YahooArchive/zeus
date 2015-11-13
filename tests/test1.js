(function() {
  var configuration = new Configuration({
    'language': 'en-US'
  });

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});

  configuration = new Configuration({
    'language': 'pt-BR'
  });

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});

  configuration = new Configuration({
    'language': 'en-US',
    'property': 'frontpage'
  });

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});

  configuration = new Configuration({
    'language': 'en-US',
    'property': 'search'
  });

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});

  configuration = new Configuration({
    'property': 'frontpage'
  });

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});

  configuration = new Configuration({});

  console.dir(configuration.provider(), {depth: null});
  console.dir(configuration.color(), {depth: null});
  console.dir(configuration.parameter(), {depth: null});
}());
