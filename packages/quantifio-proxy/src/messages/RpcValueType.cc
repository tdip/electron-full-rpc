#include "RpcValueType.h"

#include <nan.h>

#include "utils.h"

const char* REMOTE_PROPERTY_KEY = "remoteProperty";

const char* REMOTE_METHOD_KEY = "remoteMethod";

const char* REMOTE_VALUE_KEY = "remoteValue";

const char* REMOTE_OBJECT_ID_KEY = "remoteObjectId";

const char* REMOTE_PROPERTY_ID_KEY = "remotePropertyId";

const char* REMOTE_METHOD_ID_KEY = "remoteMethodId";

const char* TYPEID_KEY = "typeId";

sio::message::ptr encodeRemoteValue(v8::Local<v8::Object> object){
    utils::string remoteValue;
    if(!utils::get(TYPEID_KEY, object, remoteValue)){
        throw utils::typeError("RpcRemoteValue", object);
    }
    auto message = utils::createMessage<sio::object_message>();
    message->insert(TYPEID_KEY, *remoteValue);

    return message;
}

sio::message::ptr encodeRemoteMethod(v8::Local<v8::Object> object){
    utils::string remoteObjectId;
    utils::string remoteMethodId;

    if(
        !utils::get(REMOTE_OBJECT_ID_KEY, object, *remoteObjectId) ||
        !utils::get(REMOTE_METHOD_ID_KEY, object, *remoteMethodId)){
            throw utils::typeError("RpcRemoteMethod", object);
    }

    std::shared_ptr<sio::object_message> result = utils::createMessage<sio::object_message>();
    result->insert(REMOTE_OBJECT_ID_KEY, remoteObjectId);
    result->insert(REMOTE_METHOD_ID_KEY, remoteMethodId);

    return result;
}

sio::message::ptr encodeRemoteProperty(v8::Local<v8::Object> object){
    utils::string remoteObjectId;
    utils::string remotePropertyId;

    if(
        !utils::get(REMOTE_OBJECT_ID_KEY, object, *remoteObjectId) ||
        !utils::get(REMOTE_PROPERTY_ID_KEY, object, *remotePropertyId)){
        throw utils::typeError("RpcRemoteProperty", object);
    }

    auto result = utils::createMessage<sio::object_message>();
    result->insert(REMOTE_OBJECT_ID_KEY, *remoteObjectId);
    result->insert(REMOTE_PROPERTY_ID_KEY, *remotePropertyId);
    return result;
}

sio::message::ptr RpcValueType::Encode(v8::Local<v8::Object> object){

    auto result = utils::createMessage<sio::object_message>();
    auto remoteProperty = utils::node::to<v8::Object>(Nan::Get(
        object,
        Nan::New<v8::String>(REMOTE_PROPERTY_KEY).ToLocalChecked()));

    if(!remoteProperty.IsEmpty()){
        result->insert(
            REMOTE_PROPERTY_KEY,
            encodeRemoteProperty(remoteProperty.ToLocalChecked()));
        return result;
    }

    auto remoteMethod = utils::node::to<v8::Object>(Nan::Get(
        object,
        Nan::New<v8::String>(REMOTE_METHOD_KEY).ToLocalChecked()));
    if(!remoteMethod.IsEmpty()){
        result->insert(
            REMOTE_METHOD_KEY,
            encodeRemoteMethod(remoteMethod.ToLocalChecked()));
        return result;
    }

    auto remoteValue = utils::node::to<v8::Object>(Nan::Get(
        object,
        Nan::New<v8::String>(REMOTE_VALUE_KEY).ToLocalChecked()));
    if(!remoteValue.IsEmpty()){
        result->insert(
           REMOTE_VALUE_KEY,
            encodeRemoteValue(remoteValue.ToLocalChecked()));
        return result;
    }

    throw utils::typeError("RpcValueType", object);
}