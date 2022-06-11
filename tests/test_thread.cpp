#include "../myhttp/myhttp.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

volatile int count = 0;
//myhttp::RWMutex s_mutex;
myhttp::Mutex s_mutex;

void fun1(){
    MYHTTP_LOG_INFO(g_logger) << "name: " << myhttp::Thread::GetName()
                            << " this.name: " << myhttp::Thread::GetThis()->getName()
                            << " id: " << myhttp::GetThreadId()
                            << " this.id: " << myhttp::Thread::GetThis()->getId();
    for(int i = 0; i < 1000000; ++i){
        myhttp::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2(){
    while(true){
        MYHTTP_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3(){
    while (true)
    {
        MYHTTP_LOG_INFO(g_logger) << "====================================";
    }
}

int main(int argc, char** argv){

    YAML::Node root = YAML::LoadFile("/home/ubuntu/HttpPractice/bin/conf/test_thread.yml");
    myhttp::Config::LoadFromYaml(root);

    std::vector<myhttp::Thread::ptr> thrs;
    for(int i = 0; i < 1; ++i){
        myhttp::Thread::ptr thr(new myhttp::Thread(&fun2, "name_" + std::to_string(i * 2)));
        // myhttp::Thread::ptr thr2(new myhttp::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        // thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i){
        thrs[i]->join();
    }

    MYHTTP_LOG_INFO(g_logger) << "thread test end";
    MYHTTP_LOG_INFO(g_logger) << "count=" << count;
    
    return 0;
}