#include "RpcRemoteMethodCall.h"

#include <nan.h>
#include <vector>

#include "RpcRemoteRequest.h"
#include "utils.h"

const char* RPC_ARGS_CHANNELS_KEY = "rpcArgsChannels";
const char* RPC_RESULTS_CHANNEL_KEY = "rpcResultsChannel";
const char* RPC_REMOTE_METHOD_CALL_LABEL = "RPC_REMOTE_METHOD_CALL";

std::shared_ptr<sio::object_message> RpcRemoteMethodCall::Encode(v8::Local<v8::Object> object){
    auto message = RpcRemoteRequest::Encode(object);
    auto argsChannelsObj = utils::node::to<v8::Object>(Nan::Get(
        object,
        Nan::New<v8::String>(RPC_ARGS_CHANNELS_KEY).ToLocalChecked()));
    std::shared_ptr<std::vector<std::shared_ptr<std::string>>> argsChannels;
    if(
        argsChannelsObj.IsEmpty()
        || !argsChannelsObj.ToLocalChecked()->IsArray()
        || !utils::toArray(argsChannelsObj.ToLocalChecked().As<v8::Array>(), argsChannels)){
        throw utils::typeError("RpcRemoteMethodCall", object);
    }

    std::shared_ptr<sio::array_message> argsChannelsMessage = utils::createMessage<sio::array_message>();
    for(auto argChannel = argsChannels->begin(); argChannel != argsChannels->end(); argChannel++){
        argsChannelsMessage->push(**argChannel);
    }

    message->insert(RPC_ARGS_CHANNELS_KEY, argsChannelsMessage);

    std::shared_ptr<std::string> targetObject;
    if(!utils::get(RPC_RESULTS_CHANNEL_KEY, object, targetObject)){
        throw utils::typeError("RpcRemoteMethodCall", object);
    }
    message->insert(RPC_RESULTS_CHANNEL_KEY, *targetObject);
    return message;
}