{
  "targets": [
    {
      "target_name": "Wyd_Rupture",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "cppsrc/main.cpp", "cppsrc/Wyd_Rupture.cpp" ],
     "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}