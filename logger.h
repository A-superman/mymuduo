#pragma once

#include <string>

#include "noncopyable.h"

#define LOG_INFO(LogmsgFormat, ...)                          \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::instance();                 \
        logger.setLogLevel(INFO);                            \
        char buffer[1024] = {0};                             \
        snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buffer);                                  \
    } while (0);

#define LOG_ERROR(LogmsgFormat, ...)                         \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::instance();                 \
        logger.setLogLevel(ERROR);                           \
        char buffer[1024] = {0};                             \
        snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buffer);                                  \
    } while (0);

#define LOG_FATAL(LogmsgFormat, ...)                         \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::instance();                 \
        logger.setLogLevel(FATAL);                           \
        char buffer[1024] = {0};                             \
        snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__); \
        exit(-1);                                            \
        logger.log(buffer);                                  \
    } while (0);

#ifdef MUDEDUG
#define LOG_DEBUG(LogmsgFormat, ...)                         \
    do                                                       \
    {                                                        \
        Logger &logger = Logger::instance();                 \
        logger.setLogLevel(DEBUG);                           \
        char buffer[1024] = {0};                             \
        snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buffer);                                  \
    } while (0);
#else
#define LOG_DEBUG(LogmsgFormat, ...)
#endif

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core信息
    DEBUG, // 调试信息
};

// 输出日志类
class Logger : noncopyable
{
private:
    int logLevel_;
    Logger() {}

public:
    // 获取日志唯一的实例对象
    static Logger &instance();
    // 设置日志级别
    void setLogLevel(int Level);
    // 写日志
    void log(std::string msg);
};
