#ifndef RPC_ERROR_HH
#define RPC_ERROR_HH

#include <exception>
#include <memory>

class RpcError : public std::exception{

public:
    RpcError(const char* error){
        this->error = std::make_shared<std::string>(error);
    };

    virtual const char* what() const throw (){
        return this->error->c_str();
    }

protected:
    std::shared_ptr<std::string> error;
};

#endif