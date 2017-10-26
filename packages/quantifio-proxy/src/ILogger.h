#ifndef ILOGGER_HH
#define ILOGGER_HH

class ILogger{
public:
    virtual void Warn(const char* tag, const char* message) = 0;
    virtual void Error(const char* tag, const char* message) = 0;
};

#endif