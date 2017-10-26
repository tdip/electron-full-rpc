#ifndef RPC_MESSAGE_HH
#define RPC_MESSAGE_HH

#include <memory>
#include <string>
#include <v8.h>

#include "sio_socket.h"

/**
 * Channel through which the messages are being
 * sent.
 */
extern const char* RPC_LABEL_FIELD;

/**
 * Unique identifier for the current message 
 */
extern const char* RPC_MESSAGE_ID_FIELD;

extern const char* RPC_MESSAGE_TYPE_FIELD;

using messageContent = std::map<std::string, sio::message::ptr>;

/**
 * Represents messages that contain data related
 * to RPC calls. 
 */
class RpcMessage{
    
public:
    RpcMessage(const sio::message::ptr message);    

    /**
     * @brief The lable that indicates the group to which this message belongs.
     * To avoid sending massive chunks of data which can block an event loop
     * for a very long time, data is often sent by multiple messages. This
     * field is used to identify the group to which the messages belong.
     * 
     * @return std::string& 
     */
    const std::string& GetMessageLabel();

    const std::string& GetMessageId();

    const std::string& GetMessageType();

    const std::shared_ptr<sio::object_message> OriginalMessage(){ return std::static_pointer_cast<sio::object_message>(this->message); }

    /**
     * @brief Try parsing the message as an RpcMessage
     * 
     * @param message 
     * @return std::shared_ptr<RpcMessage> 
     */
    static const std::shared_ptr<RpcMessage> Parse(const sio::message::ptr message);

    /**
     * @brief Convert v8 objects into Socket.IO messages.
     * Converts the given object into a Socket.IO message if the object
     * matches the "RpcMessage" structure. Throws exception otherwise.
     */
    static std::shared_ptr<sio::object_message> Encode(const v8::Local<v8::Object> object);

private:
    sio::message::ptr message;
};

#endif