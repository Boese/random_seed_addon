{
  'targets': [
    {
      'target_name': 'node_rand',
      'sources': [ 'NodeRand.cc', 'NodeRandStream.cc' ],
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