#include "log.h"
#include <functional>
#include <time.h>
#include "config.h"

namespace myhttp
{

    LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char *file, int32_t line, uint32_t elapse,
                       uint32_t thread_id, uint32_t fiber_id, uint64_t time)
        : m_file(file)
        , m_line(line)
        , m_elapse(elapse)
        , m_threadId(thread_id)
        , m_fiberId(fiber_id)
        , m_time(time)
        , m_logger(logger)
        , m_level(level)
    {
    }

    void LogEvent::format(const char* fmt, ...){
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char* fmt,va_list al){
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if(len != -1){
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e)
        :m_event(e)
    {
    }

    LogEventWrap::~LogEventWrap(){
        m_event->getLogger()->log(m_event->getLevel(),m_event); // 调用该函数才开始处理 打印流程；
    }

    std::stringstream& LogEventWrap::getSS(){
        return m_event->getSS();
    }

    // ============================================LogLevel=================================================

    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
        case LogLevel::name: \
        return #name;    \
        break;
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }

    LogLevel::Level LogLevel::FromString(const std::string& str){
#define XX(name) \
        if(str == #name){ \
            return LogLevel::name; \
        }
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
        return LogLevel::UNKNOW;
#undef XX
    }
    // =============================================Logger=======================================================
    Logger::Logger(const std::string &name)
        : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::setFormatter(LogFormatter::ptr val){
        m_formatter = val;
    }

    void Logger::setFormatter(const std::string& val){
        myhttp::LogFormatter::ptr new_val(new myhttp::LogFormatter(val));
        if(new_val->isError()){
            std::cout << "Logger setFormatter name=" << m_name
                        << " value=" << val << " invalid formatter"
                        << std::endl;
            return;
        }
        m_formatter = new_val;
    }

    LogFormatter::ptr Logger::getFormatter(){
        return m_formatter;
    }


    void Logger::addAppender(LogAppender::ptr appender)
    {
        if (!appender->getFormatter())
        {
            appender->setFormatter(m_formatter);
        }

        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders(){
        m_appenders.clear();
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            if(!m_appenders.empty()){
                for (auto &i : m_appenders)
                {
                    i->log(self, level, event);
                }
            }else if(m_root){
                m_root->log(level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    // ==================================LogAppender=========================================

    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << m_formatter->format(logger, level, event);
        }
    }
    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(logger, level, event);
        }
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        reopen();
    }

    // =====================================LogFormatter===============================================

    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        init();
    }

    std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }


    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLogger()->getName();
        } 
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
            if(m_format.empty()){
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }
    };



    // %xxx ; %xxx(xxx) ; %%该字符表示转义，表示需要一个%;
    void LogFormatter::init()
    {
        // str , format , type,   vec用于存储分解后的 formatItem，用于输出内容；
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;// 用于保存格式串中，非定义字母；
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while (n < m_pattern.size())
            {
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i+1,n-i-1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if(n == m_pattern.size()){
                    if(str.empty()){
                        str = m_pattern.substr(i+1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n-1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }

        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C) \
    {#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); }}
            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(f, FilenameFormatItem),
            XX(l, LineFormatItem),
            XX(d, DateTimeFormatItem),
            XX(T, TabFormatItem),
            XX(F, FiberIdFormatItem)
#undef XX
        };

        for (auto &i : vec)
        {
            // get<idx>(container) == container[idx];
            if (std::get<2>(i) == 0) 
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << "{" << std::get<0>(i) << "} - {" << std::get<1>(i) << "} - {" << std::get<2>(i) << "}" << std::endl;
        }

        // %m -- 消息体
        // %p -- level
        // %r -- 启动后的时间
        // %c -- 日志名称
        // %t -- 线程id
        // %n -- 回车换行
        // %d -- 时间
        // %f -- 文件名
        // %l -- 行号
    }


    LoggerManager::LoggerManager(){
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
        
        //init();
    }
    Logger::ptr LoggerManager::getLogger(const std::string& name){
        auto it = m_loggers.find(name);

        if(it != m_loggers.end()){
            return it->second;
        }
        
        // 当没有找到对应的logger的时候，就构造一个，但是其内部执行m_root，相当于一个套壳；
        Logger::ptr logger(new myhttp::Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }







    struct LogAppenderDefine{
        int type = 0; // 1 File, 2 stdout
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine& oth) const{
            return type == oth.type
                && level == oth.level
                && formatter == oth.formatter
                && file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;  

        bool operator==(const LogDefine& oth) const{
            return name == oth.name
                && level == oth.level
                && formatter == oth.formatter
                && appenders == oth.appenders;
        }

        bool operator<(const LogDefine& oth) const{
            return name < oth.name;
        }
    };


    // template<>
    // class LexicalCast<std::string, myhttp::LogLevel::Level>{
    //     public:
    //         LogLevel::Level operator()(const std::string& v){
    //             YAML::Node node = YAML::Load(v);
    //             LogLevel::Level ld = LogLevel::FromString(node.as<std::string>());
    //             return ld;
    //         }
    // };
    // template<>
    // class LexicalCast<myhttp::LogLevel::Level, std::string>{
    //     public:
    //         std::string operator()(const LogLevel::Level& ld){
    //             YAML::Node node;
    //             node["level"] = LogLevel::ToString(ld);
    //             std::stringstream ss;
    //             ss << node;
    //             return ss.str();
    //         }
    // };

    template<>
    class LexicalCast<std::string, LogAppenderDefine >{
        public:
            LogAppenderDefine operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                LogAppenderDefine ld;
                ld.type = node["type"].as<int>();
                //ld.level = LexicalCast<std::string, LogLevel::Level>()(node["level"].as<std::string>());
                ld.level = LogLevel::FromString(node["level"].as<std::string>());
                ld.formatter = node["formatter"].as<std::string>();
                ld.file = node["file"].as<std::string>();
                return ld;
            }
    };
    template<>
    class LexicalCast<LogAppenderDefine, std::string>{
        public:
            std::string operator()(const LogAppenderDefine& ld){
                YAML::Node node;
                node["type"] = ld.type;
                //node["level"] = LexicalCast<LogLevel::Level, std::string>()(ld.level);
                node["level"] = LogLevel::ToString(ld.level);
                node["formatter"] = ld.formatter;
                node["file"] = ld.file;
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };
    
    template<>
    class LexicalCast<std::string, LogDefine >{
        public:
            LogDefine operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                LogDefine ld;
                ld.name = node["name"].as<std::string>();
                // ld.level = LexicalCast<std::string, LogLevel::Level>()(node["level"].as<std::string>());
                ld.level = LogLevel::FromString(node["level"].as<std::string>());
                ld.formatter = node["formatter"].as<std::string>();
                ld.appenders = LexicalCast<std::string, std::vector<LogAppenderDefine> >()(node["appenders"].as<std::string>());
                return ld;
            }
    };
    template<>
    class LexicalCast<LogDefine, std::string>{
        public:
            std::string operator()(const LogDefine& ld){
                YAML::Node node;
                node["name"] = ld.name;
                // node["level"] = LexicalCast<LogLevel::Level, std::string>()(ld.level);
                node["level"] = LogLevel::ToString(ld.level);
                node["formatter"] = ld.formatter;
                node["appenders"] = LexicalCast<std::vector<LogAppenderDefine>, std::string>()(ld.appenders);
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    // 在系统中注册一个 log配置；
    myhttp::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
        myhttp::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter{
        LogIniter() {
            // 添加 log配置 变化事件的回调函数；
            g_log_defines->addListener(0xF1E231, [](const std::set<LogDefine>& old_value,
                                                const std::set<LogDefine>& new_value){
                MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "on_logger_conf_changed";
                // 新增
                for(auto& i : new_value){
                    auto it = old_value.find(i);

                    myhttp::Logger::ptr logger;

                    if(it == old_value.end()){
                        //新增Logger；
                        logger.reset(new myhttp::Logger(i.name));
                    }else {
                        if(!( i == *it)){
                            // 修改的logger
                            logger = MYHTTP_LOG_NAME(i.name);
                        }
                    }
                    logger->setLevel(i.level);
                    if(! i.formatter.empty()){
                        logger->setFormatter(i.formatter);
                    }
                    logger->clearAppenders();
                    for(auto& a : i.appenders){
                        myhttp::LogAppender::ptr ap;
                        if(a.type == 1){
                            ap.reset(new FileLogAppender(a.file));
                        }else if(a.type == 2){
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(a.level);
                        logger->addAppender(ap);
                    }
                }

                // 删除
                for(auto& i : old_value){
                    auto it = new_value.find(i);
                    if(it == new_value.end()){
                        // 删除 logger
                        auto logger = MYHTTP_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)100);
                        logger->clearAppenders();
                     } 
                }
            });
        }
    };

    static LogIniter __log_init;
}
