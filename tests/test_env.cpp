#include "../myhttp/env.h"
#include <iostream>


// 利用/proc/pid/cmdline + 全局变量构造函数，实现在进入main函数前解析参数；

int main(int argc, char** argv){
    myhttp::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    myhttp::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    myhttp::EnvMgr::GetInstance()->addHelp("p", "print help");
    
    if(!myhttp::EnvMgr::GetInstance()->init(argc, argv)){
        myhttp::EnvMgr::GetInstance()->printHelp();
        return 0;
    }

    std::cout <<" exe=" << myhttp::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout << "cwd=" << myhttp::EnvMgr::GetInstance()->getCwd() << std::endl;

    std::cout << "path=" << myhttp::EnvMgr::GetInstance()->getEnv("PATH", "xxx") << std::endl;
    

    if(myhttp::EnvMgr::GetInstance()->has("p")){
        myhttp::EnvMgr::GetInstance()->printHelp();
    }
    return 0;
}