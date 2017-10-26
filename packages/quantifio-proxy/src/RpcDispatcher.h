#ifndef RPC_DISPATHCER_HH
#define RPC_DISPATHCER_HH

#include <mutex>
#include <nan.h>
#include <v8.h>
#include <queue>

#include "IRpcHandler.h"

using RpcDispatcherType = bool;

/**
 * Async worker that will run the Socket.io client, listen to
 * events sent via socket.io and emit events as they get pushed
 * to the event channels. Even though it is a "progress worker",
 * it will never complete it's job and the progress events wil be
 * events received via socket.io.
 */
class RpcDispatcher: public Nan::AsyncProgressWorkerBase<RpcDispatcherType>{

public:
    RpcDispatcher(Nan::Callback* done, IRpcHandler* rpc) : AsyncProgressWorkerBase<RpcDispatcherType>(done), rpc(rpc){ }

    void Execute(const Nan::AsyncProgressWorkerBase<RpcDispatcherType>::ExecutionProgress& progress);

    void HandleProgressCallback(const RpcDispatcherType *data, std::size_t count);

    void OnMessage(const sio::event &event);

    void OnDataSegmentMessage(std::shared_ptr<DataSegmentMessage> message);    

private:
    IRpcHandler* rpc;
    std::mutex eventsLock;
    std::queue<std::function<void()>> events;
};

#endif