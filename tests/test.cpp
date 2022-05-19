#include<iostream>
#include"../myhttp/log.h"

int main(int argc,char** argv){
    
    myhttp::Logger::ptr logger(new myhttp::Logger);

    logger->addAppender(myhttp::LogAppender::ptr(new myhttp::StdoutLogAppender));

    myhttp::LogEvent::ptr event(new myhttp::LogEvent(__FILE__,__LINE__,0,1,2,time(0)));

    logger->log(myhttp::LogLevel::DEBUG,event);

    std::cout << "hello world" << std::endl;
    return 0;
}