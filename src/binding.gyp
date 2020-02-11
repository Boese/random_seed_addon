{
  'targets': [
    {
      'target_name': 'random_seed',
      'sources': [ 'rand_seed.cc', 'rand_seed_stream.cc' ],
      "conditions": [['OS=="win"', {
         'msvs-settings':
          {
            'VCCLCompilerTool':
            {
              'AdditionalOptions':
                [
                '-std:c++17',
                ]
            }
          }
        }]
      ]
    }
  ],
  
  
}