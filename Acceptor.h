#pragma once
#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectCallback(const NewConnectionCallback &cb) 
    {
        newConnectionCallback_ = cb;
    }
    bool  listenning() const { return listenning_; }
    void listen();
private:
    void handleRead();
    EventLoop *loop_; // Acceptor用的就是用户定义的那个baseloop，也称作mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

