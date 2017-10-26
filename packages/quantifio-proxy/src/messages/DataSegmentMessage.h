#ifndef DATA_SEGMENT_MESSAGE_HH
#define DATA_SEGMENT_MESSAGE_HH

#include <memory>

#include "sio_socket.h"

#include "RpcMessage.h"

extern const char* DATA_SEGMENT_MESSAGE_LABEL;

extern const char* DATA_SEGMENT_METADATA_KEY;

extern const char* DATA_SEGMENT_PART_KEY;

extern const char* DATA_SEGMENT_PATH_KEY;

extern const char* DATA_SEGMENT_COMPLETE_FIELD;

using messages = std::vector<sio::message::ptr>;

class DataSegmentMessage : public RpcMessage{
public:

    DataSegmentMessage(const sio::message::ptr message): RpcMessage(message) {}

    messages& GetData();

    /**
     * @brief Indicate if the transmission of data segments has completed.
     * 
     * @return true 
     * @return false 
     */
    const bool IsComplete();

    /**
     * @brief Try parsing a message as a DataSegmentMessage.
     * Tries to parse a message as a DataSegmentMessage. Returns
     * null if the parsing fails.
     * 
     * @return std::shared_ptr<DataSegmentMessage> 
     */
    static const std::shared_ptr<DataSegmentMessage> Parse(const sio::message::ptr message);

    static std::shared_ptr<sio::object_message> Encode(v8::Local<v8::Object> object);
};
#endif