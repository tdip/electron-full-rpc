#include "RpcChannel.h"

#include <nan.h>

#include "messages/DataSegmentMessage.h"
#include "messages/RpcRemoteMethodCall.h"

template<>
NAN_METHOD(RpcChannelShared<DataSegmentMessage>::Next){

    RpcChannelWrap<DataSegmentMessage>* channel = Nan::ObjectWrap::Unwrap<RpcChannelWrap<DataSegmentMessage>>(info.Holder());
    std::shared_ptr<DataSegmentMessage> next = channel->Channel()->PopMessage();

    info.GetReturnValue().Set(Nan::New<v8::String>(next->GetMessageId().c_str()).ToLocalChecked());
}

template<>
v8::Local<v8::Value> RpcChannelSlot<DataSegmentMessage>::Convert(std::shared_ptr<DataSegmentMessage> msg) const{
    auto result = Nan::New<v8::Object>();
    return utils::node::toObject(msg->OriginalMessage());
}

template<>
v8::Local<v8::Value> RpcChannelSlot<RpcRemoteMethodCall>::Convert(std::shared_ptr<RpcRemoteMethodCall> msg) const{
    return Nan::New<v8::Object>();
}