#ifndef RPC_REMOTE_REQUEST_HH
#define RPC_REMOTE_REQUEST_HH

#include <v8.h>

#include "sio_socket.h"

class RpcRemoteRequest{
public:
    static std::shared_ptr<sio::object_message> Encode(v8::Local<v8::Object>);
};

#endif