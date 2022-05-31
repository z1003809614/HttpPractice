#ifndef __MyHTTP_LOG_H__
#define __MyHTTP_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <stdarg.h>
#include <map>
#include "singleton.h"
#include "util.h"


#define MYHTTP_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        myhttp::LogEventWrap(myhttp::LogEvent::ptr(new myhttp::LogEvent(logger,level, \
                            __FILE__, __LINE__,0, myhttp::GetThreadId(),\
                            myhttp::GetFiberId(), time(0)))).getSS() 

#define MYHTTP_LOG_DEBUG(logger) MYHTTP_LOG_LEVEL(logger, myhttp::LogLevel::DEBUG)
#define MYHTTP_LOG_INFO(logger) MYHTTP_LOG_LEVEL(logger, myhttp::LogLevel::INFO)
#define MYHTTP_LOG_WARN(logger) MYHTTP_LOG_LEVEL(logger, myhttp::LogLevel::WARN)
#define MYHTTP_LOG_ERROR(logger) MYHTTP_LOG_LEVEL(logger, myhttp::LogLevel::ERROR)
#define MYHTTP_LOG_FATAL(logger) MYHTTP_LOG_LEVEL(logger, myhttp::LogLevel::FATAL)


#define MYHTTP_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        myhttp::LogEventWrap(myhttp::LogEvent::ptr(new myhttp::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, myhttp::GetThreadId(),\
                        myhttp::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define MYHTTP_LOG_FMT_DEBUG(logger, fmt, ...) MYHTTP_LOG_FMT_LEVEL(logger, myhttp::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define MYHTTP_LOG_FMT_INFO(logger, fmt, ...) MYHTTP_LOG_FMT_LEVEL(logger, myhttp::LogLevel::INFO, fmt, __VA_ARGS__)
#define MYHTTP_LOG_FMT_WARN(logger, fmt, ...) MYHTTP_LOG_FMT_LEVEL(logger, myhttp::LogLevel::WARN, fmt, __VA_ARGS__)
#define MYHTTP_LOG_FMT_ERROR(logger, fmt, ...) MYHTTP_LOG_FMT_LEVEL(logger, myhttp::LogLevel::ERROR, fmt, __VA_ARGS__)
#define MYHTTP_LOG_FMT_FATAL(logger, fmt, ...) MYHTTP_LOG_FMT_LEVEL(logger, myhttp::LogLevel::FATAL, fmt, __VA_ARGS__)

#define MYHTTP_LOG_ROOT() myhttp::LoggerMgr::GetInstance()->getRoot()
#define MYHTTP_LOG_NAME(name) myhttp::LoggerMgr::GetInstance()->getLogger(name)

namespace myhttp
{
    class Logger;
    class LoggerManager;

    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char *ToString(LogLevel::Level Level);
        static LogLevel::Level FromString(const std::string& str);
    };

    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,
                 const char* file,int32_t line,uint32_t elapse,
                 uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint64_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time;}
        const std::string getContent() const { return m_ss.str(); }
        std::stringstream& getSS() {return m_ss;}

        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const {return m_level;} 

        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号
        uint32_t m_elapse = 0;        // 程序启动开始到现在的毫秒数
        uint32_t m_threadId;      // 线程ID
        uint32_t m_fiberId = 0;       // 携程ID
        uint64_t m_time = 0;          // 时间戳
        std::stringstream m_ss;

        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;

    };

    class LogEventWrap{
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();

        LogEvent::ptr getEvent() const { return m_event; }
        std::stringstream& getSS();

    private:
        LogEvent::ptr m_event;
    };


    // 日志格式 : 保存请求模式串(pattern)；
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);

        std::string format(std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
           
            virtual ~FormatItem() {}
            virtual void format(std::ostream &os, std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };
        // 分析patter,拆分成元组
        void init();

        bool isError() const { return m_error; }

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;

        bool m_error = false;
    };


    // 日志输出地
    class LogAppender
    {
    protected:
        /* data */
        LogLevel::Level m_level = LogLevel::DEBUG;
        LogFormatter::ptr m_formatter;

    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val;}

        LogAppender() {}
        virtual ~LogAppender() {}
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
    friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        ~Logger(){}

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        const std::string& getName() const {return m_name;}

        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);

        LogFormatter::ptr getFormatter();

    private:
        /* data */
        std::string m_name;                      //日志名称
        LogLevel::Level m_level;                 //日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合；
        LogFormatter::ptr m_formatter;
 
        Logger::ptr m_root;

    };


    // 输出到控制台的Appender
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    private:
    };

    // 定义输出到文件的Appender
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        FileLogAppender(const std::string &filename);
        bool reopen(); // 重新打开文件，文件打开成功返回true;
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };


    class LoggerManager{
    public:
        LoggerManager();
        Logger::ptr getLogger(const std::string& name);

        // void init();
        
        Logger::ptr getRoot() const {return m_root;}
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef myhttp::Singleton<LoggerManager> LoggerMgr;

}

#endif