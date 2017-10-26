#ifndef I_RPC_HANDLER_HH
#define I_RPC_HANDLER_HH

#include <memory>

#include "messages/DataSegmentMessage.h"
#include "messages/RpcRemoteMethodCall.h"

#include "ILogger.h"

extern const char * RPC_SERVER_EVENT_NAME;
extern const char * RPC_CLIENT_EVENT_NAME;

template<typename T>
class RpcChannelManager;

template<typename T>
class RpcChannel;

class IRpcHandler{
public:
    virtual std::shared_ptr<RpcChannelManager<DataSegmentMessage>> DataChannels() = 0;
    virtual std::shared_ptr<RpcChannel<sio::message>> OutgoingMessages() = 0;
    virtual std::shared_ptr<RpcChannel<RpcRemoteMethodCall>> RemoteMethodCalls() = 0;
    virtual int GetPort() = 0;
    virtual std::shared_ptr<ILogger> Logger() = 0;
};

#endif