#include "../myhttp/hook.h"
#include "../myhttp/iomanager.h"
#include "../myhttp/log.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_sleep(){
    myhttp::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        MYHTTP_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(5);
        MYHTTP_LOG_INFO(g_logger) << "sleep 3";
    });
    
    MYHTTP_LOG_INFO(g_logger) << "test_sleep";
}

int main(int argc, char** argv){
    test_sleep();
    return 0;
}
