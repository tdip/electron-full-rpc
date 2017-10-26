{
  "targets": [
    {
      "target_name": "quantifio_proxy",
      "sources": [
        "src/quantifio_proxy.cc",
        "src/RpcHandler.cc",
        "src/*.h" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "/usr/include/"
      ]
    }
  ]
}
