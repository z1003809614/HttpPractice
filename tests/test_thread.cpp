#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

volatile int count = 0;
myhttp::RWMutex s_mutex;

void fun1(){
    MYHTTP_LOG_INFO(g_logger) << "name: " << myhttp::Thread::GetName()
                            << " this.name: " << myhttp::Thread::GetThis()->getName()
                            << " id: " << myhttp::GetThreadId()
                            << " this.id: " << myhttp::Thread::GetThis()->getId();
    for(int i = 0; i < 1000000; ++i){
        myhttp::RWMutex::WriteLock lock(s_mutex);
        ++count;
    }
}

void fun2(){

}

int main(int argc, char** argv){
    std::vector<myhttp::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i){
        myhttp::Thread::ptr thr(new myhttp::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for(int i = 0; i< 5; ++i){
        thrs[i]->join();
    }

    MYHTTP_LOG_INFO(g_logger) << "thread test end";
    MYHTTP_LOG_INFO(g_logger) << "count=" << count;
    return 0;
}