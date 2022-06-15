#include "log.h"
#include <functional>
#include <time.h>
#include "config.h"

namespace myhttp
{

    LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char *file, int32_t line, uint32_t elapse,
                       uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name)
        : m_file(file)
        , m_line(line)
        , m_elapse(elapse)
        , m_threadId(thread_id)
        , m_fiberId(fiber_id)
        , m_time(time)
        , m_logger(logger)
        , m_level(level)
        , m_threadName(thread_name)
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
        std::string temp = str;
        transform(temp.begin(),temp.end(),temp.begin(),::toupper);
#define XX(name) \
        if(temp == #name){ \
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
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::setFormatter(LogFormatter::ptr val){
        MutexType::Lock lock(m_mutex);
        m_formatter = val;

        // 当logger下属的appender使用logger默认的formater的时候，进行同步更新；
        // for(auto& i : m_appenders){
        //     MutexType::Lock ll(i->m_mutex);
        //     if(!i->m_hasFormatter){
        //         i->m_formatter = m_formatter;
        //     }
        // }
    }

    void Logger::setFormatter(const std::string& val){
        myhttp::LogFormatter::ptr new_val(new myhttp::LogFormatter(val));
        if(new_val->isError()){
            std::cout << "Logger setFormatter name=" << m_name
                        << " value=" << val << " invalid formatter"
                        << std::endl;
            return;
        }
        setFormatter(new_val);
    }

    LogFormatter::ptr Logger::getFormatter(){
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }


    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        if (!appender->getFormatter())
        {
            appender->setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
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
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            MutexType::Lock lock(m_mutex);
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


    std::string Logger::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevel::ToString(m_level);
        if(m_formatter){
            node["formatter"] = m_formatter->getPattern();
        }
        for(auto& i : m_appenders){
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    // ==================================LogAppender=========================================

    void LogAppender::setFormatter(LogFormatter::ptr val) { 
        MutexType::Lock lock(m_mutex);
        m_formatter = val; 
    }
    LogFormatter::ptr LogAppender::getFormatter(){
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }


    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string StdoutLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        node["level"] = LogLevel::ToString(m_level);
        if(m_formatter){
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {   
            uint64_t now = time(0);
            // 不断重复的打开文件；效率应该会低；
            // 但是能够在log文件被删除的时候，保证重新生成文件；
            if(now != m_lastTime){ 
                reopen();
                m_lastTime = now;
            }
            MutexType::Lock lock(m_mutex);
            if(!(m_filestream << m_formatter->format(logger, level, event))){
                std::cout << "error" << std::endl;
            }
        }
    }

    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios::app);
        return !!m_filestream;
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        reopen();
    }

    std::string FileLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        node["level"] = LogLevel::ToString(m_level);
        if(m_formatter){
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
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

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadName();
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
            XX(m, MessageFormatItem),       //m:消息
            XX(p, LevelFormatItem),         //p:日志级别
            XX(r, ElapseFormatItem),        //r:累计毫秒数
            XX(c, NameFormatItem),          //c:日志名称
            XX(t, ThreadIdFormatItem),      //t:线程id
            XX(n, NewLineFormatItem),       //n:换行
            XX(f, FilenameFormatItem),      //f:文件名
            XX(l, LineFormatItem),          //l:行号
            XX(d, DateTimeFormatItem),      //d:时间
            XX(T, TabFormatItem),           //T:Tab
            XX(F, FiberIdFormatItem),       //F:协程id
            XX(N, ThreadNameFormatItem),    //N:线程名称
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
        
        m_loggers[m_root->m_name] = m_root;
        //init();
    }
    Logger::ptr LoggerManager::getLogger(const std::string& name){
        MutexType::Lock lock(m_mutex);
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

    std::string LoggerManager::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for(auto& i : m_loggers){
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
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

    template<>
    class LexicalCast<std::string, LogAppenderDefine >{
        public:
            LogAppenderDefine operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                LogAppenderDefine lad;
                if(!node["type"].IsDefined()){
                    std::cout << "log config error: appender type is null, " << node
                              << std::endl;
                    return lad;
                }
                lad.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : ""); 
                std::string type = node["type"].as<std::string>();
                if(type == "FileLogAppender"){
                    lad.type = 1;
                    if(!node["file"].IsDefined()){
                        std::cout << "log config error: fileAppender file is null, "
                                  << std::endl;
                        return lad;
                    }
                    lad.file = node["file"].as<std::string>();
                    if(node["formatter"].IsDefined()){
                        lad.formatter = node["formatter"].as<std::string>();
                    }
                }else if(type == "StdoutLogAppender"){
                    lad.type = 2;   
                }else{
                    std::cout << "log config error: appender type is invalid, "
                              << std::endl;
                }
                return lad;
            }
    };

    template<>
    class LexicalCast<LogAppenderDefine, std::string>{
        public:
            std::string operator()(const LogAppenderDefine& ld){
                YAML::Node node;
                if(ld.type == 1){
                    node["type"] = "FileLogAppender";
                    node["file"] = ld.file;
                }else if(ld.type == 2){
                    node["type"] = "StdoutLogAppender";
                }
                
                node["level"] = LogLevel::ToString(ld.level);

                if(!ld.formatter.empty()){
                    node["formatter"] = ld.formatter;
                }

                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };
    
    template<>
    class LexicalCast<std::string, LogDefine>{
        public:
            LogDefine operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                LogDefine ld;
                if(!node["name"].IsDefined()){
                    std::cout << "log config error: name is null, " << node << std::endl;
                    return ld;
                }
                ld.name = node["name"].as<std::string>();

                ld.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : ""); 
                if(node["formatter"].IsDefined()){
                    ld.formatter = node["formatter"].as<std::string>();
                }

                if(node["appenders"].IsDefined()){
                    // for(size_t x = 0; x < node["appenders"].size(); ++x){
                    //     auto a = node["appenders"][x];
                    //     LogAppenderDefine lad = LexicalCast<std::string, LogAppenderDefine>()(a.as<std::string>());
                    //     ld.appenders.push_back(lad);
                    // }
                    // 这里发现，yaml的 as 函数，似乎只能用于 scalar对象，而不能使用与 sequence对象，可能其他对象也不行，这里使用 stringstream来做string转换；-- 5/31 nxj;
                    std::stringstream ss;
                    ss << node["appenders"];
                    ld.appenders = LexicalCast<std::string, std::vector<LogAppenderDefine> >()(ss.str());
                }
                return ld;
            }
    };
    template<>
    class LexicalCast<LogDefine, std::string>{
        public:
            std::string operator()(const LogDefine& ld){
                YAML::Node node;
                node["name"] = ld.name;
                node["level"] = LogLevel::ToString(ld.level);
                if(!ld.formatter.empty()){
                    node["formatter"] = ld.formatter;
                }
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
            // 回调函数执行完之前，全局变量执行的数据还没改变，所以，在回调中打印，依旧无数值 -- 5/31 nxj；
            g_log_defines->addListener([](const std::set<LogDefine>& old_value,
                                                const std::set<LogDefine>& new_value){
                MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "on_logger_conf_changed";
                // 新增
                for(auto& i : new_value){
                    auto it = old_value.find(i);

                    myhttp::Logger::ptr logger;

                    if(it == old_value.end()){
                        //新增Logger；
                        // 这种写法，会导致新logger没有注册到logMgr中；
                        // logger.reset(new myhttp::Logger(i.name)); 
                        logger = MYHTTP_LOG_NAME(i.name);
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

                        if(!a.formatter.empty()){
                            LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                            if(!fmt->isError()){
                                ap->setFormatter(fmt);
                            }else{
                                std::cout << "log name=" << i.name << "appender type=" << a.type 
                                          << " formatter=" << a.formatter << " is invalid" << std::endl;
                            }
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
