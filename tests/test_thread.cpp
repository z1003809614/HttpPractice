#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void fun1(){
    MYHTTP_LOG_INFO(g_logger) << "name: " << myhttp::Thread::GetName()
                            << " this.name: " << myhttp::Thread::GetThis()->getName()
                            << " id: " << myhttp::GetThreadId()
                            << " this.id: " << myhttp::Thread::GetThis()->getId();
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

    return 0;
}