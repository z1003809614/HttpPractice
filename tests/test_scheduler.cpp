#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_fiber(){
    static int s_count = 5;
    MYHTTP_LOG_INFO(g_logger) << " test in fiber s_count=" << s_count; 
    if(--s_count >= 0){
        myhttp::Scheduler::GetThis()->schedule(&test_fiber, myhttp::GetThreadId());
    }
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

