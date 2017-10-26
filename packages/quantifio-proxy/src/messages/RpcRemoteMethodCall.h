#ifndef RPC_REMOTE_METHOD_CALL
#define RPC_REMOTE_METHOD_CALL

#include "RpcRemoteRequest.h"

class RpcRemoteMethodCall{
public:
    static std::shared_ptr<sio::object_message> Encode(v8::Local<v8::Object>);
};

#endif