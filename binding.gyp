{
  'targets': [
    {
      'target_name': 'zoom',
      'cflags': [],
      'libraries': [],
      'dependencies': [
        './deps/yaz/yaz.gyp:yaz'
      ],
      'sources': [
        'src/zoom.cc',
        'src/record.cc',
        'src/resultset.cc',
        'src/query.cc',
        'src/scanset.cc',
        'src/connection.cc'
      ]
    }
  ]
}
