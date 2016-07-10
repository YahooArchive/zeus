
$configuration = new Configuration(array(
  'language' => 'en-US',
));

var_dump($configuration->provider, $configuration->color, $configuration->parameter);

$configuration = new Configuration(array(
  'language' => 'pt-BR',
));

var_dump($configuration->provider, $configuration->color, $configuration->parameter);

$configuration = new Configuration(array(
  'language' => 'en-US',
  'property' => 'frontpage',
));

var_dump($configuration->provider, $configuration->color, $configuration->parameter);

$configuration = new Configuration(array(
  'language' => 'en-US',
  'property' => 'search',
));

var_dump($configuration->provider, $configuration->color, $configuration->parameter);

$configuration = new Configuration(array(
  'property' => 'frontpage',
));

var_dump($configuration->provider, $configuration->color, $configuration->parameter);

$configuration = new Configuration(array());

var_dump($configuration->provider, $configuration->color, $configuration->parameter);
