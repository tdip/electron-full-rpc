#include "RpcHandler.h";

#include <nan.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <v8.h>

Nan::Persistent<v8::Function> RpcHandler::constructor;

NAN_METHOD(Notify){

    auto messageObj = info[0];
    if(!messageObj->IsString()){
        return;
    }
    usleep(1000000);
    auto message = messageObj->ToString();
    auto isolate = info.GetIsolate();
    RpcHandler* rpc = Nan::GetIsolateData<RpcHandler>(isolate);
    char messageVals[message->Utf8Length()];
    message->WriteUtf8(messageVals);
    rpc->PushMessage(messageVals);
}

std::shared_ptr<std::string> RpcHandler::PopMessage(){
    std::shared_ptr<std::string> result;
    while(true){
        pthread_mutex_lock(&this->mutex);
        if(this->messages.size() > 0){
            result = this->messages.front();
            this->messages.erase(this->messages.begin());
            break;
        }
        pthread_mutex_unlock(&this->mutex);
        usleep(1000);
    }

    return result;
}

void RpcHandler::PushMessage(char* message){
    pthread_mutex_lock(&this->mutex);
    this->messages.push_back(std::make_shared<std::string>(message));
    pthread_mutex_unlock(&this->mutex);
}

static void* messageDispatcher(void* args){

    // Create an isolated context for the
    // dispatcher thread
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(params);
    
    v8::Locker wtf(isolate);
    
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

    RpcHandler* rpc = (RpcHandler*) args;

    FILE* fp = fopen(rpc->supportModule->c_str(), "rb");
    fseek(fp, 0, SEEK_END);    
    long len = ftell(fp);
    rewind(fp);
    char* buf = (char*) calloc(len + 1, sizeof(char));
    fread(buf, len, 1, fp);
    fclose(fp);

    v8::Local<v8::Value> result = v8::Script::Compile(v8::String::NewFromUtf8(isolate, buf))->Run();
    free(buf);
    auto dispatch = context->Global()->Get(v8::String::NewFromUtf8(isolate, "dispatcher"));

    if(!dispatch->IsFunction()){
        auto obj = dispatch->ToDetailString()->ToString();
        char details [obj->Utf8Length()];
        obj->WriteUtf8(details);
        char * test = details;
        pthread_mutex_lock(&rpc->mutex);
        rpc->error = std::make_shared<std::string>("The dispatcher script must define a global function called 'dispatcher'.");
        pthread_mutex_unlock(&rpc->mutex);
        return NULL;
    }

    Nan::SetIsolateData(isolate, rpc);
    v8::Local<v8::Object> notifier = Nan::New<v8::Object>();
    Nan::SetMethod(notifier, "notify", Notify);

    v8::Local<v8::Value> argv[0];
    Nan::To<v8::Function>(dispatch).ToLocalChecked()->Call(notifier, 0, argv);

    return NULL;
}

NAN_METHOD(RpcHandler::New){
    auto isolate = v8::Isolate::GetCurrent();
    auto supportModule = info[0];
    v8::Local<v8::Script> script = v8::Script::Compile(v8::String::NewFromUtf8(isolate, "console.log('kaisi')"));
    
    if(supportModule->IsUndefined() || !supportModule->IsString()){
        Nan::ThrowError(v8::String::NewFromUtf8(isolate, "Constructor requires one argument"));
    }

    if(info.IsConstructCall()){
        
        auto supportString = supportModule->ToString();
        char string[supportString->Utf8Length()];
        supportString->WriteUtf8(string);

        auto obj = new RpcHandler(string);
        
        pthread_create(&obj->thread, NULL, messageDispatcher, obj);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }

    Nan::ThrowError(v8::String::NewFromUtf8(isolate, "The 'RpcHandler' must be used with the new keyword"));
}

NAN_METHOD(RpcHandler::CallRemoteMethod){
    RpcHandler* rpc = ObjectWrap::Unwrap<RpcHandler>(info.Holder());
    auto message = rpc->PopMessage();
    info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), message->c_str()));
}

NAN_METHOD(RpcHandler::GetStatus){
    auto isolate = v8::Isolate::GetCurrent();
    RpcHandler* rpc = ObjectWrap::Unwrap<RpcHandler>(info.Holder());
    
    if(rpc->error != NULL){

        auto result = v8::String::NewFromUtf8(isolate, rpc->error->c_str());
        info.GetReturnValue().Set(result);     
    }    
}

NAN_MODULE_INIT(RpcHandler::Init){
    
    Nan::HandleScope scope;    

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RpcHandler").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "getStatus", GetStatus);
    Nan::SetPrototypeMethod(tpl, "callRemoteMethod", CallRemoteMethod);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    target->Set(
        Nan::New("RpcHandler").ToLocalChecked(),
        Nan::GetFunction(tpl).ToLocalChecked());
}