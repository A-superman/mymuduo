#include <iostream>

#include "logger.h"
#include "Timestamp.h"

// 获取日志唯一的实例对象
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}
// 设置日志级别
void Logger::setLogLevel(int Level) {
    logLevel_ = Level;
}
/*
INFO,  // 普通信息
ERROR, // 错误信息
FATAL, // core信息
DEBUG, // 调试信息
*/
// 写日志
void Logger::log(std::string msg) {
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }

    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}