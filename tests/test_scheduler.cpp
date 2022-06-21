#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_fiber(){
    MYHTTP_LOG_INFO(g_logger) << " test in fiber";
    sleep(1); 
    myhttp::Scheduler::GetThis()->schedule(&test_fiber);
}

int main(int argc, char** argv){
    MYHTTP_LOG_INFO(g_logger) << " main";
    myhttp::Scheduler sc(3, false);
    sc.start();
    MYHTTP_LOG_INFO(g_logger) << " schedule ";
    sc.schedule(&test_fiber);
    sc.stop();
    MYHTTP_LOG_INFO(g_logger) << "over";
    return 0;
}

