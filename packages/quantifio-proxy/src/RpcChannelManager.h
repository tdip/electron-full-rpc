#ifndef RPC_CHANNEL_MANAGER_HH
#define RPC_CHANNEL_MANAGER_HH

#include <map>
#include <mutex>

#include "RpcChannel.h"
#include "utils.h"

template<class T>
class RpcChannelManager{
public:
    std::shared_ptr<RpcChannel<T>> UseChannel(std::shared_ptr<std::string> channelId){
        std::shared_ptr<RpcChannel<T>> result = this->channels[channelId];
    
        if(result == NULL){
            std::lock_guard<std::mutex> lk(this->mutex);
            result = this->channels[channelId];
            if(result == NULL){
                result = std::make_shared<RpcChannel<T>>();
                this->channels[channelId] = result;
            }
        }
    
        return result;
    }

    void CloseChannel(const std::string& channelId){
        std::lock_guard<std::mutex> lk(this->mutex);
        this->channels.erase(channelId);
    }

private:
    std::mutex mutex;
    std::map<std::shared_ptr<std::string>, std::shared_ptr<RpcChannel<T>>, utils::ptr_less<std::string>> channels;
};

#endif