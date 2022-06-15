#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void run_in_fiber(){
    MYHTTP_LOG_INFO(g_logger) << "run_in_fiber begin";
    myhttp::Fiber::YieldToHold();
    MYHTTP_LOG_INFO(g_logger) << "run_in_fiber end";
}

void test_fiber(){
    // 创建主协程；
    myhttp::Fiber::GetThis();
    MYHTTP_LOG_INFO(g_logger) << "main begin";
    myhttp::Fiber::ptr fiber(new myhttp::Fiber(run_in_fiber));
    fiber->swapIn();
    MYHTTP_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();
    MYHTTP_LOG_INFO(g_logger) << "main after end";
}

int main(int argc, char** argv){
    myhttp::Thread::SetName("main");
    
    std::vector<myhttp::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i){
        thrs.push_back(myhttp::Thread::ptr(new myhttp::Thread(&test_fiber,"name_" + std::to_string(i))));
    }

    for(auto i : thrs){
        i->join();
    }

    return 0;
}