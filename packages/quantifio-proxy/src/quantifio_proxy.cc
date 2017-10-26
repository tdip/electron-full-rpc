#include <nan.h>
#include <stdio.h>
#include <v8.h>

#include "RpcHandler.h"

NAN_METHOD(RpcInvoke){
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    auto location = isolate->GetCallingContext()->Global()->GetPropertyNames();
    info.GetReturnValue().Set(location);
}

NAN_MODULE_INIT(InitAll){
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    NAN_EXPORT(target, RpcInvoke);
    RpcHandler::Init(target);
}

NODE_MODULE(quantifio_proxy, InitAll)