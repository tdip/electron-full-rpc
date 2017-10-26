#include  "RpcMessage.h"

#include "utils.h"

const char* RPC_LABEL_FIELD = "rpcLabel";

const char* RPC_MESSAGE_ID_FIELD = "rpcMessageId";

const char* RPC_MESSAGE_TYPE_FIELD = "rpcMessageType";

RpcMessage::RpcMessage(const sio::message::ptr message) {
    this->message = message;
}

const std::string& RpcMessage::GetMessageType(){
    return this->OriginalMessage()->get_map()[RPC_MESSAGE_TYPE_FIELD]->get_string();
}

const std::string& RpcMessage::GetMessageId(){
    return this->OriginalMessage()->get_map()[RPC_MESSAGE_ID_FIELD]->get_string();
}

const std::string& RpcMessage::GetMessageLabel(){
    return this->OriginalMessage()->get_map()[RPC_LABEL_FIELD]->get_string();
}

const std::shared_ptr<RpcMessage> RpcMessage::Parse(const sio::message::ptr message){
    if(message->get_flag() != sio::message::flag::flag_object){
        return NULL;
    }

    auto map = message->get_map();

    if(
        map[RPC_LABEL_FIELD] == NULL
        || map[RPC_LABEL_FIELD]->get_flag() != sio::message::flag_string
        || map[RPC_MESSAGE_ID_FIELD] == NULL
        || map[RPC_MESSAGE_ID_FIELD]->get_flag() != sio::message::flag_string
        || map[RPC_MESSAGE_TYPE_FIELD] == NULL
        || map[RPC_MESSAGE_TYPE_FIELD]->get_flag() != sio::message::flag_string
    ){
        return NULL;
    }

    return std::make_shared<RpcMessage>(message);
}

std::shared_ptr<sio::object_message> RpcMessage::Encode(const v8::Local<v8::Object> object){
    utils::string label;
    utils::string messageId;
    utils::string messageType;

    if(
        !utils::get<utils::string>(RPC_LABEL_FIELD, object, label) ||
        !utils::get<utils::string>(RPC_MESSAGE_ID_FIELD, object, messageId) ||
        !utils::get<utils::string>(RPC_MESSAGE_TYPE_FIELD, object, messageType)
    ){
        throw utils::typeError("RpcMessage", object);
    }

    auto message = utils::createMessage<sio::object_message>();
    message->insert(RPC_LABEL_FIELD, label->c_str());
    message->insert(RPC_MESSAGE_ID_FIELD, messageId->c_str());
    message->insert(RPC_MESSAGE_TYPE_FIELD, messageType->c_str());

    return message;
}