#ifndef RPC_HANDLER_HH
#define RPC_HANDLER_HH

#include <nan.h>
#include <pthread.h>
#include <v8.h>
#include <vector>

class RpcHandler : public Nan::ObjectWrap{
public:
    static NAN_MODULE_INIT(Init);

    pthread_mutex_t mutex;
    
    pthread_t thread;
    
    std::shared_ptr<std::string> supportModule;
    
    std::shared_ptr<std::string> error;

    void PushMessage(char* message);

    std::shared_ptr<std::string> PopMessage();

private:
    RpcHandler(char* support) {
        this->supportModule = std::make_shared<std::string>(support);
        pthread_mutex_init(&this->mutex, NULL);
    }

    static NAN_METHOD(New);
    
    static Nan::Persistent<v8::Function> constructor;
    
    static NAN_METHOD(GetStatus);

    static NAN_METHOD(CallRemoteMethod);

    std::vector<std::shared_ptr<std::string>> messages;
};

#endif