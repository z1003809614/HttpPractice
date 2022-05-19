#include<iostream>
#include"../myhttp/log.h"
#include"../myhttp/util.h"

int main(int argc,char** argv){
    // logger : 分发event到不同的Appender中, 其输出格式 保存在 formatter中，logger内部包含了一个formatter的指针，且 目前的格式 存储静态存储在代码中； 
    // 处理流程：formatter 分析 pattern -》 分解得到不同的 FormatItem ->  appender -> 执行不同FormatItem的打印行为 -> 打印数据来自event 或 logger中；    
    myhttp::Logger::ptr logger(new myhttp::Logger);

    logger->addAppender(myhttp::LogAppender::ptr(new myhttp::StdoutLogAppender));

    // myhttp::LogEvent::ptr event(new myhttp::LogEvent(__FILE__,__LINE__,0,std::this_thread::get_id(), myhttp::GetFiberID(),time(0)));    

    // logger->log(myhttp::LogLevel::DEBUG,event);

    MYHTTP_LOG_INFO(logger) << "test macro";

    std::cout << myhttp::GetThreadId() << std::endl;
    std::cout << "hello world" << std::endl;
    return 0;
}