#include "../myhttp/daemon.h"
#include "../myhttp/iomanager.h"
#include "../myhttp/log.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

myhttp::Timer::ptr timer;
int server_main(int argc, char** argv){
    MYHTTP_LOG_INFO(g_logger) << myhttp::ProcessInfoMgr::GetInstance()->toString();
    myhttp::IOManager iom(1);
    timer = iom.addTimer(1000, [](){
        MYHTTP_LOG_INFO(g_logger) << "onTimer";
        static int count = 0;
        if(++count > 10){
            timer->cancel();
        }
    }, true);
    return 0;
}

int main(int argc, char** argv){
    return myhttp::start_daemon(argc, argv, server_main, argc!=1);
}