#include "RpcDispatcher.h"

#include <sio_client.h>
#include <sio_socket.h>
#include <sstream>

#include "RpcChannelManager.h"

void RpcDispatcher::OnDataSegmentMessage(const std::shared_ptr<DataSegmentMessage> message){
    rpc->Logger()->Warn("OnDataSegmentMessage", "Pushing message");
    std::shared_ptr<std::string> label = std::make_shared<std::string>(message->GetMessageLabel());
    auto channel = this->rpc->DataChannels()->UseChannel(label);
    auto event = channel->PushMessage(message);


    // If this was the last message of the message group
    // we notify the channel that no more messages will be
    // arriving
    //if(message->IsComplete()){
        //channel->SetDone();
    // }
    std::lock_guard<std::mutex>(this->eventsLock);
    this->events.push(std::move(event));
}

void RpcDispatcher::HandleProgressCallback(const RpcDispatcherType*, std::size_t){
    while(this->events.size() > 0){
        std::lock_guard<std::mutex>(this->eventsLock);
        this->events.front()();
        this->events.pop();
    }
}

void RpcDispatcher::OnMessage(const sio::event &event){
    std::stringstream msg;
    auto rpc = this->rpc;
    const std::shared_ptr<sio::message> payload = event.get_message();

    rpc->Logger()->Warn("OnMessage", "Getting message.");

    if(payload->get_flag() != sio::message::flag_array){
        rpc->Logger()->Warn("OnMessage", "Invaild message.");
        return;
    }

    auto vec = payload->get_vector();

    for(auto&& it = vec.begin(); it != vec.end(); it++){
        const std::shared_ptr<DataSegmentMessage> dsm = DataSegmentMessage::Parse(*it);
        if(dsm != NULL){
            this->OnDataSegmentMessage(dsm);
        }else{
            rpc->Logger()->Warn("OnMessage", "Cannot decode message.");
        }
    }
}

void RpcDispatcher::Execute(const Nan::AsyncProgressWorkerBase<RpcDispatcherType>::ExecutionProgress& progress){
    std::stringstream hostname;
    auto rpc = this->rpc;
    const bool data = true;    
    hostname << "http://localhost:" << rpc->GetPort();
    sio::client client;
    client.socket()->on(RPC_CLIENT_EVENT_NAME, [&data, this, &progress](sio::event &event){
        this->OnMessage(event);
        progress.Send(&data, 1);
    });

    client.connect(hostname.str().c_str());

    // Start the loop that fetches messages
    // from the outgoing message queue.
    std::shared_ptr<sio::message> msg;
    for(;;){
        // Wait for a message to become available
        msg = rpc->OutgoingMessages()->PopMessage();
        if(msg == NULL){
            break;
        }
        client.socket()->emit(RPC_SERVER_EVENT_NAME, msg);
    };

    client.sync_close();
}