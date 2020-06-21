{
  'defines': [
    "NAPI_VERSION=4"
  ],
  'targets': [
    {
      'target_name': 'node_rand',
      'sources': [ 'NodeRand.cc', 'NodeRandStream.cc' ],
      "conditions": [['OS=="win"', {
         'msvs_settings':
          {
            'VCCLCompilerTool':
            {
              'AdditionalOptions':
                [
                '/std:c++17',
                ]
            }
          }
        }],
        ['OS=="linux"', {
         'cflags_cc+': [
           '-std=c++17'
         ]
        }
        ]
      ]
    }
  ],
  
  
}