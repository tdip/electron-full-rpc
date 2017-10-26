#include <nan.h>
#include <stdio.h>
#include <v8.h>

#include "RpcHandler.h";

const char* MESSAGE_DISPATCHER_SCRIPT = "dispatcher.js";

void MessageDispatcher(){

    FILE* fp = fopen(MESSAGE_DISPATCHER_SCRIPT, "r");
    fseek(fp, 0, SEEK_END);
    long len= ftell(fp);
    rewind(fp);
    char* buf = (char*) calloc(len + 1, sizeof(char));
    fread(buf, len, 1, fp);
    fclose(fp);
    const v8::Isolate::CreateParams args;
    v8::Isolate* isolate = v8::Isolate::New(args);

    v8::Local<v8::Script> script = v8::Script::Compile(v8::String::NewFromUtf8(isolate, buf));
    auto result = script->Run();
    //result->ToObject()->CallAsFunction(isolate->GetCurrentContext(), 0, NULL);
}

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