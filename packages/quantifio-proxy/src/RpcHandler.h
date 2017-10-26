#ifndef RPC_HANDLER_HH
#define RPC_HANDLER_HH

#include <mutex>
#include <nan.h>
#include <thread>
#include <v8.h>
#include <vector>

#include "sio_socket.h"

#include "messages/DataSegmentMessage.h"

#include "ILogger.h"
#include "IRpcHandler.h"
#include "RpcChannelManager.h"
#include "RpcDispatcher.h"

class RpcHandler : public Nan::ObjectWrap, public IRpcHandler{
public:
    static NAN_MODULE_INIT(Init);
        
    std::shared_ptr<std::string> error;

    int GetPort();

    ~RpcHandler();

    std::shared_ptr<RpcChannelManager<DataSegmentMessage>> DataChannels() override {return this->dataChannels;}

    std::shared_ptr<RpcChannel<sio::message>> OutgoingMessages() override { return this->outgoingMessages; }

    std::shared_ptr<RpcChannel<RpcRemoteMethodCall>> RemoteMethodCalls() override { return this->remoteMethodCallsChannel; }

    std::shared_ptr<ILogger> Logger(){ return this->logger; }

private:
    RpcHandler(int32_t port);

    std::shared_ptr<RpcChannelManager<DataSegmentMessage>> dataChannels;
    
    std::shared_ptr<RpcChannel<RpcRemoteMethodCall>> remoteMethodCallsChannel;

    std::shared_ptr<RpcChannel<sio::message>> outgoingMessages;

    int32_t port;

    std::shared_ptr<RpcDispatcher> dispatcher;

    static NAN_METHOD(New);
    
    static Nan::Persistent<v8::Function> constructor;
    
    static NAN_METHOD(GetStatus);

    static NAN_METHOD(CallRemoteMethod);

    static NAN_METHOD(EmitRpcCall);

    static NAN_METHOD(EmitData);

    static NAN_METHOD(GetData);

    static void MessageDispatcher(RpcHandler* rpc);

    std::vector<std::shared_ptr<std::string>> messages;

    std::shared_ptr<ILogger> logger;    

    /**
     * This is the constructor that allows creating javascript
     * instances of rpc channels that deliver RPC related
     * messages.
     */
    static Nan::Persistent<v8::Function> RpcChannelConstructor;

    /**
     * Constructor to wrap the RpcMethodCall requests channel.
    */
    static Nan::Persistent<v8::Function> RpcRemoteMethodChannelConstructor;
};

#endif