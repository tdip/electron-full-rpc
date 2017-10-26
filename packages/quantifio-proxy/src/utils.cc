#include "utils.h"

#include <nan.h>
#include <sstream>

std::shared_ptr<std::string> utils::getString(v8::Local<v8::String> v8String){
    char content[v8String->Utf8Length()];
    v8String->WriteUtf8(content);
    return std::make_shared<std::string>((char*)content);
}

const utils::string utils::printObject(v8::Local<v8::Value> obj){
    return utils::getString(obj->ToDetailString());
}

RpcError utils::typeError(const char* ty, v8::Local<v8::Object> object){
    std::stringstream message;
    auto details = utils::printObject(object);
    message << "Could not convert '" << details->c_str() << "' to '" << ty << "'.";
    return RpcError(message.str().c_str());
}

RpcError encodeError(v8::Local<v8::Value> object){
    std::stringstream message;
    auto details = utils::printObject(object);
    message << "Could not convert '" << details->c_str() <<"' to Socket.IO message";
    return RpcError(message.str().c_str());
}

template<>
const bool utils::is<v8::Object>(Nan::MaybeLocal<v8::Value>& value){
    return !value.IsEmpty() && value.ToLocalChecked()->IsObject();
}

template<>
const bool utils::is<v8::Array>(Nan::MaybeLocal<v8::Value>& value){
    return !value.IsEmpty() && value.ToLocalChecked()->IsArray();
}

template<>
const bool utils::is<v8::String>(Nan::MaybeLocal<v8::Value>& value){
    return !value.IsEmpty() && value.ToLocalChecked()->IsString();
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> object, v8::Local<v8::Value>& result){

    auto value = Nan::Get(object, Nan::New<v8::String>(key).ToLocalChecked());
    if(value.IsEmpty()){
        return false;
    }

    result = value.ToLocalChecked();
    return true;
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> object, v8::Local<v8::Object>& result){
    v8::Local<v8::Value> value;
    if(utils::get(key, object, value)){
        return false;
    }

    Nan::MaybeLocal<v8::Object> _object = Nan::To<v8::Object>(value);

    if(_object.IsEmpty()){
        return false;
    }

    result = _object.ToLocalChecked();

    return true;
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> object, v8::Local<v8::Array>& result){
    v8::Local<v8::Value> value;

    if(!utils::get(key, object, value) || !value->IsArray()){
        return false;
    }

    result = value.As<v8::Array>();
    return true;
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> obj, std::shared_ptr<std::string>& result){

    auto value = Nan::Get(
        obj,
        Nan::New<v8::String>(key).ToLocalChecked());

    if(
        value.IsEmpty() ||
        !value.ToLocalChecked()->IsString()
    ){
        return false;
    }

    result = utils::getString(value.ToLocalChecked()->ToString());
    return true;
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> obj, int32_t& result){

    return false;
}

template<>
const bool utils::get(const char* key, v8::Local<v8::Object> obj, bool& result){
    auto value = Nan::Get(
        obj,
        Nan::New<v8::String>(key).ToLocalChecked());

    if(value.IsEmpty() || !value.ToLocalChecked()->IsBoolean()){
        return false;
    }

    result = value.ToLocalChecked()->ToBoolean()->BooleanValue();
    return true;
}

template<> sio::message::ptr utils::toMessage(v8::Local<v8::Object> object);
template<> sio::message::ptr utils::toMessage(v8::Local<v8::String> string);
template<> sio::message::ptr utils::toMessage(v8::Local<v8::Array> string);

template<>
sio::message::ptr utils::toMessage(v8::Local<v8::Value> value){
    
    if(value->IsString()){
        return toMessage(value->ToString());
    }

    if(value->IsArray()){
        return toMessage(value.As<v8::Array>());
    }
    
    if(value->IsObject()){
        return toMessage(value->ToObject());
    }

    throw encodeError(value);
}

template<>
sio::message::ptr utils::toMessage(v8::Local<v8::Array> array){
    std::shared_ptr<sio::array_message> result = std::static_pointer_cast<sio::array_message>(sio::array_message::create());

    for(uint32_t i = 0; i < array->Length(); i++){
        auto value = Nan::Get(array, i);

        // Todo: check that value is defined and crash gracefully
        // if not

        result->insert(i, toMessage(value.ToLocalChecked()));
    }

    return result;
}

template<>
sio::message::ptr utils::toMessage(v8::Local<v8::Object> object){
    auto properties = object->GetOwnPropertyNames();
    std::shared_ptr<sio::object_message> result = std::static_pointer_cast<sio::object_message>(sio::object_message::create());

    for(uint32_t i = 0; i < properties->Length(); i++){
        auto key = properties->Get(i);
        auto value = Nan::Get(object, key);
        
        // Todo: check that value is defined and crash gracefully
        // if not.

        result->insert(
            getString(key->ToString())->c_str(),
            toMessage(value.ToLocalChecked())   
        );
    }

    return result;
}

template<>
sio::message::ptr utils::toMessage(v8::Local<v8::String> string){
    return sio::string_message::create(getString(string)->c_str());
}

template<>
std::shared_ptr<sio::object_message> utils::createMessage(){
    return std::static_pointer_cast<sio::object_message>(sio::object_message::create());
}

template<>
std::shared_ptr<sio::array_message> utils::createMessage(){
    return std::static_pointer_cast<sio::array_message>(sio::array_message::create());
}

template<>
const bool utils::toArray(const v8::Local<v8::Array> array, std::shared_ptr<std::vector<utils::string>>& result){

    result = std::make_shared<std::vector<utils::string>>();
    result->reserve(array->Length());

    for(uint32_t i = 0; i < array->Length(); i++){
        auto value = Nan::Get(array, i);
        if(utils::is<v8::String>(value)){
            result->push_back(utils::getString(value.ToLocalChecked()->ToString()));
        }else{
            return false;
        }
    }

    return true;
}

v8::Local<v8::Array> messageToObject(const std::shared_ptr<sio::array_message> message){
    auto array = Nan::New<v8::Array>();
    auto items = message->get_vector();

    for(uint32_t i = 0; i < items.size(); i++){
        Nan::Set(array, i, utils::node::toObject(items[i]));
    }

    return array;
}

v8::Local<v8::Object> messageToObject(const std::shared_ptr<sio::object_message> message){
    auto object = Nan::New<v8::Object>();
    auto mappings = message->get_map();

    for(auto item = mappings.begin(); item != mappings.end(); item++){
        Nan::Set(
            object,
            Nan::New<v8::String>(item->first).ToLocalChecked(),
            utils::node::toObject(item->second));
    }
    return object;
}

v8::Local<v8::Value> utils::node::toObject(const sio::message::ptr message){

    //Todo: optimize and remove recursion
    switch(message->get_flag()){
        case sio::message::flag_array:
            return messageToObject(std::static_pointer_cast<sio::array_message>(message));
        case sio::message::flag_boolean:
            return Nan::New<v8::Boolean>(message->get_bool());
        case sio::message::flag_double:
            return Nan::New<v8::Number>(message->get_double());
        case sio::message::flag_integer:
            return Nan::New<v8::Number>(message->get_int());
        case sio::message::flag_null:
            return Nan::Null();
        case sio::message::flag_object:
            return messageToObject(std::static_pointer_cast<sio::object_message>(message));
        case sio::message::flag_string:
            return Nan::New<v8::String>(message->get_string()).ToLocalChecked();
    }

    throw RpcError("Binary messages cannot be converted to object.");
}
