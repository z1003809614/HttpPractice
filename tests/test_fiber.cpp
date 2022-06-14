#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void run_in_fiber(){
    MYHTTP_LOG_INFO(g_logger) << "run_in_fiber begin";
    myhttp::Fiber::YieldToHold();
    MYHTTP_LOG_INFO(g_logger) << "run_in_fiber end";
    myhttp::Fiber::YieldToHold();
}

int main(int argc, char** argv){
    myhttp::Fiber::GetThis();
    MYHTTP_LOG_INFO(g_logger) << "main begin";
    myhttp::Fiber::ptr fiber(new myhttp::Fiber(run_in_fiber));
    fiber->swapIn();
    MYHTTP_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();
    MYHTTP_LOG_INFO(g_logger) << "main after end";
    return 0;
}