var http = require('http').createServer();
var path = require('path');
var io = require('socket.io')(http);

var port = 3000;

http.listen(port);

var proxy = require('./build/Release/quantifio_proxy');
console.log(path.resolve('./dispatcher.js'));
var h = new proxy.RpcHandler(port);

const obj = {
    rpcLabel: "rpcLabel",
    rpcMessageId: "rpcMessageId",
    rpcMessageType: "DATA_SEGMENT_MESSAGE_TYPE",
    part: "",
    path: ["soy", "kaiser"],
    metadata: {
        type: {
            remoteValue: {typeId: "typeId"}
        }
    }
};

h.emitData(obj);

(function messenger(){
    h.emitData(obj);
    setTimeout(messenger, 1000);
})();

console.log(proxy.constants.RpcServerEvent);
var x = 42;

h.getData("rpcLabel").onMessage(msg => {
    console.log("ON messagggeee", msg, x);
});

io.on('connection', socket => {
    socket.on(proxy.constants.RpcServerEvent, (data) => {
        console.log("emitted", JSON.stringify(data));
    });

    socket.emit(proxy.constants.RpcClientEvent, [obj]);
});

/*
function loader(){
    
    setTimeout(loader, 1100);
};

setTimeout(loader, 1100);
*/