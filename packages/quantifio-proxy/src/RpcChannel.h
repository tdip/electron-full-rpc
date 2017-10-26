#ifndef RPC_CHANNEL_HH
#define RPC_CHANNEL_HH

#include <boost/signals2.hpp>
#include <condition_variable>
#include <future>
#include <mutex>
#include <nan.h>
#include <thread>
#include <v8.h>
#include <vector>

#include  "messages/DataSegmentMessage.h"
#include "IRpcHandler.h"
#include "utils.h"

template<typename T>
using bSignal = boost::signals2::signal<T>;

using bConn = boost::signals2::connection;

template<typename T>
class RpcChannel{
public:
    RpcChannel(){ }
    
    std::function<void()> PushMessage(std::shared_ptr<T> message){
        std::unique_lock<std::mutex> lk(this->mutex);
        this->messages.push_back(message);
        this->emptyLock.notify_one();
        return [this](){this->Signal();};
    }

    void PushMessages(std::vector<std::shared_ptr<T>> messages){
        std::unique_lock<std::mutex> lk(this->mutex);
        for(auto message = messages.begin(); message != messages.end(); message++){
            this->messages.push_back(message);
            this->onMessageSignal(message);
            this->emptyLock.notify_one();
        }
    }
    
    void SetDone(){
        std::unique_lock<std::mutex> lk(this->mutex);
        this->done = true;
        this->emptyLock.notify_all();
    }

    /**
     * @brief Signal the subscribed slots if messages are queued.
     * RpcChannels can be used synchronously and asynchronously. This
     * methods will send an async signal to all slots subscribed to this
     * channel if there are messages in the channel. 
     */
    void Signal(){
        std::shared_ptr<T> msg;
        {
            std::unique_lock<std::mutex> lk(this->mutex);
            if(this->messages.size() > 0){
                msg = this->messages.front();
                this->messages.erase(this->messages.begin());
            }
        }

        if(msg != NULL){
            this->onMessageSignal(msg);
        }
    }
    
    std::shared_ptr<T> PopMessage(){
        std::unique_lock<std::mutex> lk(this->mutex);
    
        if(this->done && this->messages.size() == 0){
            return NULL;
        }
    
        // Wait for messages to arrive
        if(this->messages.size() == 0){
            this->emptyLock.wait(lk);
        }
    
        // If the emptyLock is released and there
        // are no values in messages, it means that
        // the channel is done receiving values
        // so we return null.
        if(this->messages.size() == 0){
            return NULL;
        }
    
        auto result = this->messages.front();
        this->messages.erase(this->messages.begin());
        return result;
    }

    bSignal<void (std::shared_ptr<T>)>& OnMessageSignal(){
        return this->onMessageSignal;
    }

private:
    std::mutex mutex;

    bSignal<void (std::shared_ptr<T>)> onMessageSignal;

    /**
     * If the channel has no messages but it is still
     * not finished getting all the messages it is
     * expected to receive. This lock will become
     * active until a new message is received.
     */
    std::condition_variable emptyLock;
    std::vector<std::shared_ptr<T>> messages;
    bool done = false;

    void UnlockEmptyIfLocked();
};

template<typename T>
class RpcChannelSlot{
public:

    RpcChannelSlot(std::shared_ptr<Nan::Callback> cb): cb(cb) {}

    void operator()(std::shared_ptr<T> value) const{
        Nan::HandleScope scope;
        v8::Local<v8::Value> args[] = {
            this->Convert(value)
        };
        this->cb->Call(1, args);
    }
private:
    std::shared_ptr<Nan::Callback> cb;
    v8::Isolate* iso;
    v8::Local<v8::Value> Convert(std::shared_ptr<T> value) const;
};

template<typename T>
class RpcChannelSubscription{
public:
    RpcChannelSubscription(
        std::shared_ptr<RpcChannel<T>> channel,
        std::shared_ptr<Nan::Callback> cb)
        : connection(channel->OnMessageSignal().connect(RpcChannelSlot<T>(cb))){ }

private:
    bConn connection;
};

template<typename T>
class RpcChannelWrap;

template<typename T>
class RpcChannelShared{
public:
    static NAN_METHOD(Next);

    static NAN_METHOD(OnMessage){
        
        auto cb = info[0];

        if(!cb->IsFunction()){
            Nan::ThrowError(Nan::Error("'OnMessage' requires a function on it's first argument."));
        }
        
        RpcChannelWrap<T>* channel = Nan::ObjectWrap::Unwrap<RpcChannelWrap<T>>(info.Holder());
        auto handler = std::make_shared<Nan::Callback>(cb.As<v8::Function>());
        new RpcChannelSubscription<T>(
            channel->Channel(),
            handler);
        info.GetReturnValue().SetUndefined();
    }
};

template<typename T>
class RpcChannelWrap: public Nan::ObjectWrap{

public:
    RpcChannelWrap(std::shared_ptr<RpcChannel<T>> channel, v8::Local<v8::Object> obj): channel(channel){
        this->Wrap(obj);
    }

    static NAN_METHOD(New){
        auto isolate = v8::Isolate::GetCurrent();
        auto rpcObj = info[0];
    
        if(!rpcObj->IsObject()){
            Nan::ThrowError(Nan::TypeError("RpcChannelWrap requires an 'RpcHandler' as firt argument."));
            return;
        }
    
        try{
            std::shared_ptr<RpcChannel<T>> channel = RpcChannelWrap<T>::GetChannel(rpcObj.As<v8::Object>(), info);
            auto obj = info.Holder();
            auto wrap = new RpcChannelWrap<T>(channel, obj->ToObject());
            //Nan::SetMethod(obj, "next", RpcChannelShared<DataSegmentMessage>::Next);
            //Nan::SetMethod(obj, "onMessage", RpcChannelShared<DataSegmentMessage>::OnMessage);
            info.GetReturnValue().Set(obj);
        }catch(RpcError& e){
            Nan::ThrowError(Nan::Error(e.what()));
        }
    }

    std::shared_ptr<RpcChannel<T>> Channel(){
        return this->channel;
    }

private:
    std::shared_ptr<RpcChannel<T>> channel;
    static std::shared_ptr<RpcChannel<T>> GetChannel(v8::Local<v8::Object>, Nan::NAN_METHOD_ARGS_TYPE info);
};

#endif