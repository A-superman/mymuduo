#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb
    , const std::string &name) 
    : callback_(cb)
    , thread_(std::bind(&EventLoopThread::threadFunc, this))
    , loop_(nullptr)
    , exiting_(false) 
    , cond_()
    , mutex_()
{

}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr) 
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层的新线程
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr) 
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}
// 在单独的新线程中运行
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个eventloop，和上面的线程是一一对应的，one loop per thread
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop(); // Eventloop loop => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}