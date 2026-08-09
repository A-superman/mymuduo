#include "Channel.h"
#include "logger.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

// EventLoop: ChannelList Poller
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) 
{
}
Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}
/*
* 当改变channel所表示fd的events事件后，updata负责在poller里面更改fd相应的事件epoll_ctl
* EventLoop => ChannelList  Poller
*/
void Channel::update() {
    // 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
    // add code...
    loop_->updateChannel(this);
}
// 在channel所属的Eventloop中删除当前channel
void Channel::remove() {
    loop_->removeChannel(this);
}

// fd得到poller通知以后，处理事件的
void Channel::handleEvent(Timestamp receiveTime) {
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    
    LOG_INFO("channel handleEvents revents:%d\n", revents_);

    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallback_) {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR) {
        if(errorCallback_) {
            errorCallback_();
        }
    }
    if(revents_ & KReadEvent) {
        if(readCallback_) {
            readCallback_(receiveTime);
        }
    }
    if(revents_ & KWriteEvent) {
        if(writeCallback_) {
            writeCallback_();
        }
    }
}
