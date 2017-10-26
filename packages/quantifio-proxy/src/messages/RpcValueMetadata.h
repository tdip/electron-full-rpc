#ifndef DATA_SEGMENT_CHUNK_HH
#define DATA_SEGMENT_CHUNK_HH

#include <v8.h>

#include "sio_socket.h"

extern const char* METADATA_TYPE_KEY;

class RpcValueMetadata{
public:
    static sio::message::ptr Encode(v8::Local<v8::Object> object);
};

#endif