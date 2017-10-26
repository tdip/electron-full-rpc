#include "RpcHandler.h"

#include <memory>
#include <nan.h>
#include <node.h>
#include <sstream>
#include <thread>
#include <v8.h>

#include "sio_client.h"
#include "sio_socket.h"

#include "errors/RpcError.h"

#include "messages/DataSegmentMessage.h"
#include "messages/RpcRemoteMethodCall.h"

#include "ILogger.h"
#include "utils.h"

Nan::Persistent<v8::Function> RpcHandler::constructor;
Nan::Persistent<v8::Function> RpcHandler::RpcChannelConstructor;
Nan::Persistent<v8::Function> RpcRemoteMethodChannelConstructor;

class DebugLogger : public ILogger{
public:
    DebugLogger(): ILogger(){}

    void Warn(const char* tag, const char* message) override{
        printf("Warn '%s': %s \n", tag, message);
    }

    void Error(const char* tag, const char* message) override{
        printf("Error '%s': %s\n", tag, message);
    }    
};

NAN_METHOD(CloseDispatcher){
}

RpcHandler::RpcHandler(int32_t port) :
    port(port){
    this->dataChannels = std::make_shared<RpcChannelManager<DataSegmentMessage>>();
    this->logger = std::make_shared<DebugLogger>();
    this->outgoingMessages = std::make_shared<RpcChannel<sio::message>>();
    this->dispatcher = std::make_shared<RpcDispatcher>(
        new Nan::Callback(Nan::New<v8::Function>(CloseDispatcher)),
        this
    );
    Nan::AsyncQueueWorker(this->dispatcher.get());
 }

 RpcHandler::~RpcHandler(){

    // Notify the Socket.IO client that it should
    // stop listening.
     this->outgoingMessages->SetDone();
}

int32_t RpcHandler::GetPort(){
    return this->port;
}

NAN_METHOD(RpcHandler::New){
    auto isolate = v8::Isolate::GetCurrent();
    auto supportModule = info[0];
    v8::Local<v8::Script> script = v8::Script::Compile(v8::String::NewFromUtf8(isolate, "console.log('kaisi')"));
    
    if(supportModule->IsUndefined() || !supportModule->IsNumber()){
        Nan::ThrowError(v8::String::NewFromUtf8(isolate, "Constructor requires one number argument"));
    }

    if(info.IsConstructCall()){
        
        int32_t port = supportModule->ToNumber()->Uint32Value();

        auto obj = new RpcHandler(port);
                
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }

    Nan::ThrowError(v8::String::NewFromUtf8(isolate, "The 'RpcHandler' must be used with the new keyword"));
}

NAN_METHOD(RpcHandler::EmitRpcCall){
    RpcHandler* rpc = ObjectWrap::Unwrap<RpcHandler>(info.Holder());
    auto rpcMethodCallMessage = utils::node::to<v8::Object>(info[0]);

    if(rpcMethodCallMessage.IsEmpty()){
        Nan::ThrowError(Nan::Error("'EmitRpcCall' requires an 'RpcMethodCallMessage' as first argument."));
    }

    try{
        auto message = RpcRemoteMethodCall::Encode(rpcMethodCallMessage.ToLocalChecked());
        rpc->outgoingMessages->PushMessage(message);
        info.GetReturnValue().SetUndefined();
    }catch(RpcError& error){
        Nan::ThrowError(Nan::Error(error.what()));
    }
}

NAN_METHOD(RpcHandler::GetData){

    auto iso = info.GetIsolate();
    // Check that a channelId  is given so we can know what channel we should
    // listen to.
    auto channelId = info[0];
    if(!channelId->IsString()){
        Nan::ThrowError(Nan::TypeError("'GetData' requires a string on it's first argument."));
    }

    v8::Local<v8::Value> args[2] = { info.Holder(), channelId };
    auto result = Nan::CallAsConstructor(RpcHandler::RpcChannelConstructor.Get(iso), 2, args)
        .ToLocalChecked();
    info.GetReturnValue().Set(result);
}

NAN_METHOD(RpcHandler::EmitData){
    RpcHandler* rpc = Nan::ObjectWrap::Unwrap<RpcHandler>(info.Holder());
    auto data = utils::node::to<v8::Object>(info[0]);
    if(data.IsEmpty()){
        Nan::ThrowError(Nan::Error("Emit data requires its first argument to be an object"));
    }

    try{
        auto message = DataSegmentMessage::Encode(data.ToLocalChecked());
        rpc->outgoingMessages->PushMessage(message);
        info.GetReturnValue().SetUndefined();
    }catch(RpcError& error){
        Nan::ThrowError(Nan::Error(error.what()));
    }
}

NAN_METHOD(RpcHandler::GetStatus){
    auto isolate = v8::Isolate::GetCurrent();
    RpcHandler* rpc = ObjectWrap::Unwrap<RpcHandler>(info.Holder());
    
    if(rpc->error != NULL){

        auto result = v8::String::NewFromUtf8(isolate, rpc->error->c_str());
        info.GetReturnValue().Set(result);     
    }    
}

NAN_MODULE_INIT(RpcHandler::Init){
    
    Nan::HandleScope scope;

    // Define the constants exposed by this module
    v8::Local<v8::Object> constants = Nan::New<v8::Object>();

    Nan::Set(
        constants,
        Nan::New<v8::String>("RpcServerEvent").ToLocalChecked(),
        Nan::New<v8::String>(RPC_SERVER_EVENT_NAME).ToLocalChecked());

    Nan::Set(
        constants,
        Nan::New<v8::String>("RpcClientEvent").ToLocalChecked(),
        Nan::New<v8::String>(RPC_CLIENT_EVENT_NAME).ToLocalChecked());

    Nan::Set(
        target,
        Nan::New<v8::String>("constants").ToLocalChecked(),
        constants);

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RpcHandler").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "emitData", EmitData);
    Nan::SetPrototypeMethod(tpl, "getData", GetData);
    Nan::SetPrototypeMethod(tpl, "getStatus", GetStatus);
    Nan::SetPrototypeMethod(tpl, "emitRpcCall", EmitRpcCall);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    target->Set(
        Nan::New("RpcHandler").ToLocalChecked(),
        Nan::GetFunction(tpl).ToLocalChecked());

    // Create the consturctor for the RpcChannel Objects
    v8::Local<v8::FunctionTemplate> channelTpl = Nan::New<v8::FunctionTemplate>(RpcChannelWrap<DataSegmentMessage>::New);
    channelTpl->SetClassName(Nan::New("RpcChannel").ToLocalChecked());
    channelTpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(channelTpl, "next", RpcChannelShared<DataSegmentMessage>::Next);
    Nan::SetPrototypeMethod(channelTpl, "onMessage", RpcChannelShared<DataSegmentMessage>::OnMessage);
    RpcHandler::RpcChannelConstructor.Reset(Nan::GetFunction(channelTpl).ToLocalChecked());

    // Create the constructor for the RpcRemoteMethodCall channel
    v8::Local<v8::FunctionTemplate> remoteMethodTpl = Nan::New<v8::FunctionTemplate>(RpcChannelWrap<RpcRemoteMethodCall>::New);
    remoteMethodTpl->SetClassName(Nan::New("RpcRemoteMethodChannel").ToLocalChecked());
    remoteMethodTpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(remoteMethodTpl, "onMessage", RpcChannelShared<RpcRemoteMethodCall>::OnMessage);
}

template<>
std::shared_ptr<RpcChannel<DataSegmentMessage>> RpcChannelWrap<DataSegmentMessage>::GetChannel(v8::Local<v8::Object> rpcObj, Nan::NAN_METHOD_ARGS_TYPE info){
    auto channelIdObj = info[1];
    
    if(!channelIdObj->IsString()){
        throw RpcError("'RpcChannel' needs a channel id to be created");
    }
    RpcHandler* rpc = Nan::ObjectWrap::Unwrap<RpcHandler>(rpcObj);
    auto channelId = utils::getString(channelIdObj->ToString());    
    
    return rpc->DataChannels()->UseChannel(channelId);
}

template<>
std::shared_ptr<RpcChannel<RpcRemoteMethodCall>> RpcChannelWrap<RpcRemoteMethodCall>::GetChannel(v8::Local<v8::Object> rpcObj, Nan::NAN_METHOD_ARGS_TYPE info){
    RpcHandler* rpc = Nan::ObjectWrap::Unwrap<RpcHandler>(rpcObj);    
    return rpc->RemoteMethodCalls();
}