#include "DataSegmentMessage.h"

#include <vector>

#include "RpcValueMetadata.h"
#include "utils.h"

const char* DATA_SEGMENT_MESSAGE_TYPE = "DATA_SEGMENT_MESSAGE_TYPE";

const char* DATA_SEGMENT_METADATA_KEY = "metadata";

const char* DATA_SEGMENT_PART_KEY = "part";

const char* DATA_SEGMENT_PATH_KEY = "path";

const char* DATA_SEGMENT_COMPLETE_FIELD = "complete";

const bool DataSegmentMessage::IsComplete(){
    auto ptr = this->OriginalMessage();
    auto message = *ptr;

    return message.has(DATA_SEGMENT_COMPLETE_FIELD) &&
        message[DATA_SEGMENT_COMPLETE_FIELD]->get_bool();
}

static bool isDataSegmentType(const std::string& type){
    return type.compare(DATA_SEGMENT_MESSAGE_TYPE) == 0;
}

const std::shared_ptr<DataSegmentMessage> DataSegmentMessage::Parse(const sio::message::ptr message){

    auto rpcMessage = RpcMessage::Parse(message);
    
    if(rpcMessage == NULL){
        return NULL;
    }

    auto ptr = rpcMessage->OriginalMessage();
    auto content = *ptr;

    // Todo: validate the metadata section
    if(
        // Ensure the type matches DataSegmentMessage
        rpcMessage->GetMessageType().compare(DATA_SEGMENT_MESSAGE_TYPE) != 0

        // Ensure the message has a path
        || !content.has(DATA_SEGMENT_PATH_KEY)

        // If the message has a complete field, ensure it is a boolean
        || (content.has(DATA_SEGMENT_COMPLETE_FIELD) &&
            content[DATA_SEGMENT_COMPLETE_FIELD]->get_flag() != sio::message::flag_boolean)

        // If the message has a part field, ensure it is a string
        || (content.has(DATA_SEGMENT_PART_KEY) &&
            content[DATA_SEGMENT_PART_KEY]->get_flag() != sio::message::flag_string)
    ){
        return NULL;
    }

    return std::make_shared<DataSegmentMessage>(message);
}

std::shared_ptr<sio::object_message> DataSegmentMessage::Encode(v8::Local<v8::Object> object){
    auto message = RpcMessage::Encode(object);
    if(
        !utils::validate(RPC_MESSAGE_TYPE_FIELD, message, isDataSegmentType)){
        throw utils::typeError("DataSegmentMessage", object);
    }

    // Extract the path from the object provided
    v8::Local<v8::Array> pathArray;
    std::shared_ptr<std::vector<utils::string>> path;
    if(
        utils::get(DATA_SEGMENT_PATH_KEY, object, pathArray)
        && utils::toArray(pathArray, path)){
        auto pathMessage = utils::createMessage<sio::array_message>();
        for(auto item = path->begin(); item != path->end(); item++){
            pathMessage->push(**item);
        }
        message->insert(DATA_SEGMENT_PATH_KEY, pathMessage);
    }else{
        throw utils::typeError("DataSegmentMessage", object);
    }

    // Check if object contains a data part and extract
    // the data as string if exists. This is stored
    // as a string because the Object.parse method
    // will be called once we are back in javascript
    utils::string part;
    if(utils::get(DATA_SEGMENT_PART_KEY, object, part)){
        message->insert(DATA_SEGMENT_PART_KEY, *part);
    }

    // Check if object contains a metadata section and
    // store it in the rpc message
    auto meta = utils::node::to<v8::Object>(Nan::Get(
        object,
        Nan::New<v8::String>(DATA_SEGMENT_METADATA_KEY).ToLocalChecked()));
    if(!meta.IsEmpty()){
        message->insert(
            DATA_SEGMENT_METADATA_KEY,
            RpcValueMetadata::Encode(meta.ToLocalChecked()));
    }

    return message;
}