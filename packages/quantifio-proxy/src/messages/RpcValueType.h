#ifndef RPC_REMOTE_VALUE_HH
#define RPC_REMOTE_VALUE_HH

#include "v8.h"

#include "sio_socket.h"

extern const char* REMOTE_PROPERTY_KEY;

extern const char* REMOTE_VALUE_KEY;

extern const char* REMOTE_METHOD_KEY;

class RpcValueType{
public:
    static sio::message::ptr Encode(v8::Local<v8::Object> object);    
};

#endif