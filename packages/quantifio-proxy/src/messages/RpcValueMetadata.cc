#include "RpcValueMetadata.h"

#include "RpcValueType.h"
#include "utils.h"

const char* METADATA_TYPE_KEY = "type";

sio::message::ptr RpcValueMetadata::Encode(v8::Local<v8::Object> object){

    auto typeObj = utils::node::to<v8::Object>(
        Nan::Get(
            object,
            Nan::New<v8::String>(METADATA_TYPE_KEY).ToLocalChecked()));
    if(typeObj.IsEmpty()){
        throw utils::typeError("RpcValueMetadata", object);
    }

    std::shared_ptr<sio::object_message> result = utils::createMessage<sio::object_message>();
    result->insert(METADATA_TYPE_KEY, RpcValueType::Encode(typeObj.ToLocalChecked()));
    return result;
}