#include<iostream>
#include"../myhttp/log.h"
#include"../myhttp/util.h"

int main(int argc,char** argv){
    // logger : 分发event到不同的Appender中, 其输出格式 保存在 formatter中，logger内部包含了一个formatter的指针，且 目前的格式 存储静态存储在代码中； 
    // 处理流程：formatter 分析 pattern -》 分解得到不同的 FormatItem ->  appender -> 执行不同FormatItem的打印行为 -> 打印数据来自event 或 logger中；    
    myhttp::Logger::ptr logger(new myhttp::Logger);

    logger->addAppender(myhttp::LogAppender::ptr(new myhttp::StdoutLogAppender));

    // 第四次测试样例： 测试FileLogAppender是否正常；
    myhttp::FileLogAppender::ptr file_appender(new myhttp::FileLogAppender("./log.txt"));
    logger->addAppender(file_appender);
    file_appender->setLevel(myhttp::LogLevel::ERROR);

    // 第5次测试样例： 测试自定义fmt是否正常；
    myhttp::LogFormatter::ptr fmt(new myhttp::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt); // Logger默认会构造一个 formatter，如果appender的formatter为空，则把自己的传给appender；

    // 第一次测试样例；
    // myhttp::LogEvent::ptr event(new myhttp::LogEvent(__FILE__,__LINE__,0,std::this_thread::get_id(), myhttp::GetFiberID(),time(0)));    
    // logger->log(myhttp::LogLevel::DEBUG,event);

    // 第二次测试样例；
    // 宏会定义一个 临时LogEventWrap对象，调用getSS函数，返回一个stringstream类，然后执行 <<重载，将后面的字符串加入到返回的流中，
    // 然后该临时对象生命周期结束，然后就执行析构函数，在其内部调用了 logger——》log函数，执行了 打印流程；
    MYHTTP_LOG_INFO(logger) << "test macro"; // 该宏返回的是一个stringstream类；
    MYHTTP_LOG_ERROR(logger) << "test macro error";

    // 第三次测试样例； // 宏返回的stringsteam 与 pattern 无关；下面的字符串 是 pattern中%m 解析后需要输出的内容；
    // 如果pattern没有%m，则下面的字符串不会输出；
    MYHTTP_LOG_FMT_DEBUG(logger, " test macro fmt error %s", "aa");

    // auto l = myhttp::LoggerMgr::GetInstance()->getLogger("xx");
    // MYHTTP_LOG_INFO(l) << "xxx";

    return 0;
}