项目：mymuduo（简化版 muduo 学习实现）
=================================

概览
--
这是一个基于 epoll 的简易网络库，实现了事件循环（EventLoop）、Poller、Channel、
以及 TCP 层封装（TcpServer / TcpConnection）。用于学习网络库设计与 I/O 多路复用。

目录结构（主要）：
- code/                : 源代码目录
  - Channel.h/.cc      : 封装 fd 与事件及回调
  - EventLoop.h/.cc    : 事件循环
  - Poller.h           : Poller 抽象接口
  - EPollPoller.h/.cc  : 基于 epoll 的 Poller 实现
  - TcpServer.h/.cc    : 服务器逻辑（accept -> TcpConnection）
  - TcpConnection.h/.cc: 单个连接封装（socket + channel + buffer）
  - Buffer.h/.cc       : 缓冲区读写封装
  - Socket.h/.cc       : socket 封装
  - InetAddress.*      : 地址封装
  - logger.*           : 日志
  - example/           : 示例程序（如 testserver）

核心设计要点
- 一个 `EventLoop` 对应一个线程，`TcpServer` 在 main loop 上 accept，新连接分配到 sub-loop；
- `Channel` 负责注册 fd 的事件和调用具体回调（read/write/close/error）；
- `EPollPoller` 负责把 `Channel` 的 events 注册到 epoll，并把活跃的 `Channel` 填入 `EventLoop` 的列表；
- `TcpConnection` 把 `Socket`、`Channel`、`Buffer` 组合在一起，封装连接状态机与读写逻辑；

构建与运行
--
在仓库根目录下执行：

```bash
mkdir -p build
cd build
cmake ../code
make -j4
```

详细执行流程（testserver）
--
下面按调用链逐步说明 `example/testserver` 启动和处理一次客户端请求时的函数流向，便于你在源码中对照查找：

1. `main()`（[code/example/testserver.cc](code/example/testserver.cc#L23)）
  - 创建 `EventLoop loop;` 与 `EchoServer server(&loop, addr, name)`，调用 `server.start()`。
  - 调用 `loop.loop()` 进入事件循环。

2. `TcpServer::start()`（[code/TcpServer.cc](code/TcpServer.cc)）
  - 启动 `Acceptor`：创建 `Channel` 监听 `listenfd` 的 `EPOLLIN`，并将 `accept` 的回调注册到主 `EventLoop`。
  - 启动 `EventLoopThreadPool`（若已设置线程数），准备 sub-loop 以处理连接

3. 内核检测到新连接 -> `Acceptor::handleRead()`（[code/Acceptor.cc](code/Acceptor.cc)）
  - 调用 `accept()` 得到 `connfd`，并调用 `TcpServer::newConnection(connfd, peerAddr)`。

4. `TcpServer::newConnection()`（[code/TcpServer.cc](code/TcpServer.cc)）
  - 选取一个 `EventLoop`（主 loop 或一个 sub-loop）作为该连接的 owner loop。
  - 创建 `std::shared_ptr<TcpConnection>`：
     - 内部创建 `Socket` 与 `Channel`（`Channel(thisLoop, connfd)`）
     - 为 `Channel` 设置回调：`setReadCallback(bind(&TcpConnection::handleRead, this, _1))`，以及 write/close/error 回调
     - 调用 `channel->tie(shared_from_this())`，防止异步回调中对象被销毁
  - 在该 owner loop 调用 `TcpConnection::connectEstablished()` 完成注册

5. `TcpConnection::connectEstablished()`（[code/TcpConnection.cc](code/TcpConnection.cc)）
  - `channel_->enableReading()` 将 channel 的 `events_` 注册到 `EPollPoller`（`updateChannel` -> `epoll_ctl(ADD)`）
  - 调用用户层 `connectionCallback_`（`EchoServer::onConnection`），通知连接已建立

6. 事件循环：`EventLoop::loop()` 调用 `Poller::poll()`（[code/EventLoop.cc](code/EventLoop.cc)）
  - `EPollPoller::poll()` 使用 `epoll_wait`，返回活跃事件列表
  - `EPollPoller::fillActiveChannels()` 为每个 `epoll_event` 设置对应 `Channel::revents_` 并 push 到 `activeChannels`

7. `EventLoop` 处理活跃 `Channel` 列表（[code/EventLoop.cc](code/EventLoop.cc)）
  - 遍历 `activeChannels`，调用 `channel->handleEvent(pollReturnTime)`

8. `Channel::handleEventWithGuard()`（[code/Channel.cc](code/Channel.cc)）
  - 根据 `revents_` 判断并调用：
     - `readCallback_(Timestamp)`（-> `TcpConnection::handleRead`）
     - `writeCallback_()`（-> `TcpConnection::handleWrite`）
     - `closeCallback_()`（-> `TcpConnection::handleClose`）
     - `errorCallback_()`

9. `TcpConnection::handleRead(Timestamp)`（[code/TcpConnection.cc](code/TcpConnection.cc)）
  - 使用 `inputBuffer_.readFd(fd, &savedErrno)` 从 socket 读取数据；
  - `n > 0`：调用用户 `messageCallback_`（`EchoServer::onMessage`）处理数据；
  - `n == 0`：对端关闭，调用 `handleClose()`；
  - `n < 0`：出错，调用 `handleError()`。

10. `EchoServer::onMessage()`（[code/example/testserver.cc](code/example/testserver.cc#L1)）
   - 从 `Buffer` 取出消息 `retrieveAllAsString()`；
   - 调用 `conn->send(msg)` 发送回显；
   - 调用 `conn->shutdown()` 请求关闭写端（示例中立即关闭，触发写完后走关闭流程）。

11. `TcpConnection::sendInLoop()`（[code/TcpConnection.cc](code/TcpConnection.cc)）
   - 若当前没有注册写事件且 `outputBuffer_` 为空，尝试 `write()`；
   - 若写不完，将剩余数据 append 到 `outputBuffer_` 并 `channel_->enableWriting()` 注册 `EPOLLOUT`。

12. `TcpConnection::handleWrite()`（[code/TcpConnection.cc](code/TcpConnection.cc)）
   - 当内核通知 `EPOLLOUT`，写出 `outputBuffer_` 的数据；写完后 `disableWriting()`；
   - 若 `state_ == kDisconnecting` 且写完，调用 `shutdownInLoop()` 完成写端关闭。

13. `TcpConnection::handleClose()`（[code/TcpConnection.cc](code/TcpConnection.cc)）
   - `setState(kDisconnected)`，`channel_->disableAll()`；
   - 调用 `connectionCallback_(connPtr)`（通知断开）与 `closeCallback_(connPtr)`（`TcpServer` 用以移除 connection）；
   - `channel_->remove()` 从 `EPollPoller` 中删除该 channel，最终 `shared_ptr` 释放导致析构。

运行示例（在另一个终端用 `nc` 测试）：

```bash
cd ../code/example
./testserver
# 另一个终端
nc 127.0.0.1 8000
```