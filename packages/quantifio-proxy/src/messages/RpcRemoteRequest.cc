#include "RpcRemoteRequest.h"

#include "RpcMessage.h"
#include "utils.h"

const char* RPC_TARGET_CLIENT_KEY = "RPC_TARGET_CLIENT_KEY";
const char* RPC_TARGET_OBJECT_KEY = "RPC_TARGET_OBJECT_KEY";

std::shared_ptr<sio::object_message> RpcRemoteRequest::Encode(v8::Local<v8::Object> object){
    auto message = RpcMessage::Encode(object);
    utils::string targetClient;
    utils::string targetObject;

    if(
        !utils::get(RPC_TARGET_CLIENT_KEY, object, targetClient) ||
        !utils::get(RPC_TARGET_OBJECT_KEY, object, targetObject)){
        throw utils::typeError("RpcRemoteRequest", object);
    }

    message->insert(RPC_TARGET_CLIENT_KEY, *targetClient);
    message->insert(RPC_TARGET_OBJECT_KEY, *targetObject);

    return message;
}