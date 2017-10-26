#ifndef UTILS_HH
#define UTILS_HH

#include <memory>
#include <nan.h>
#include <sio_socket.h>
#include <v8.h>
#include <vector>

#include "errors/RpcError.h"

namespace utils{

    typedef std::shared_ptr<std::string> string;

    std::shared_ptr<std::string> getString(v8::Local<v8::String> v8String);

    template<class T> struct ptr_less {
        bool operator()(std::shared_ptr<T> lhs, std::shared_ptr<T> rhs) {
            std::less<T> cmp;
            return cmp(*lhs, *rhs);
        }
    };

    template<typename T>
    const bool get(const char* key, v8::Local<v8::Object> obj, T& result);

    
    template<typename F>
    const bool validate(const char* key, sio::message::ptr message, F&& assertion){
        return message->get_flag() == sio::message::flag_object
            && std::static_pointer_cast<sio::object_message>(message)->has(key)
            && message->get_map()[key]->get_flag() == sio::message::flag_string
            && assertion(message->get_map()[key]->get_string());
    }

    template<typename T>
    sio::message::ptr toMessage(v8::Local<T>);

    const string printObject(v8::Local<v8::Value>);

    RpcError typeError(const char*, v8::Local<v8::Object>);

    template<typename T>
    std::shared_ptr<T> createMessage();

    template<typename T>
    const bool is(Nan::MaybeLocal<v8::Value>&);

    template<typename T>
    const bool toArray(const v8::Local<v8::Array>, std::shared_ptr<std::vector<T>>&);

    namespace node{
        template<typename T>
        Nan::MaybeLocal<T> to(Nan::MaybeLocal<v8::Value> value){
            if(value.IsEmpty() || value.ToLocalChecked()->IsUndefined()){
                return Nan::MaybeLocal<T>();
            }else{
                return Nan::To<T>(value.ToLocalChecked());
            }
        }

        template<typename T>
        Nan::MaybeLocal<T> get(v8::Local<v8::Value> value, const char* key){
            return to<T>(Nan::Get(
                value,
                Nan::New<v8::String>(key).ToLocalChecked()
            ));
        }

        template<typename T>
        v8::Local<v8::String> str(T&);

        v8::Local<v8::Value> toObject(const sio::message::ptr);
    }
}

#endif