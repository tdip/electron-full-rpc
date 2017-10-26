var path = require('path');
var proxy = require('./build/Release/quantifio_proxy');
console.log(path.resolve('./dispatcher.js'));
var h = new proxy.RpcHandler(path.resolve('./dispatcher.js'));

console.log(h.callRemoteMethod());

setTimeout(() => console.log(h.getStatus()), 300);