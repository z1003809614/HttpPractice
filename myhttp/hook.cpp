#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"

#include <dlfcn.h>
namespace myhttp
{
    static thread_local bool t_hook_enable = false;

    #define HOOK_FUN(XX) \
        XX(sleep) \
        XX(usleep)
    
    void hook_init(){
        static bool is_inited = false;
        if(is_inited){
            return;
        }

    #define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
    #undef XX
    }

    struct _HookIniter
    {
        _HookIniter(){
            hook_init();
        }
    };
    // 保证其在正式程序执行之前能够初始化；
    static _HookIniter s_hook_initer;

    bool is_hook_enable(){
        return t_hook_enable;
    }   
    void set_hook_enable(bool flag){
        t_hook_enable = flag;
    } 
} // namespace myhttp

extern "C"{
    #define XX(name) name ## _fun name ## _f = nullptr;
        HOOK_FUN(XX)
    #undef XX

        
    unsigned int sleep(unsigned int seconds){
        if(!myhttp::t_hook_enable) {
            return sleep_f(seconds);
        }

        myhttp::Fiber::ptr fiber = myhttp::Fiber::GetThis();
        myhttp::IOManager* iom = myhttp::IOManager::GetThis();
        // iom->addTimer(seconds * 1000, std::bind(&myhttp::IOManager::schedule, iom, fiber));
        iom->addTimer(seconds * 1000, [iom, fiber](){
            iom->schedule(fiber);
        });

        myhttp::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec){
        if(!myhttp::t_hook_enable){
            return usleep_f(usec);
        }
        myhttp::Fiber::ptr fiber = myhttp::Fiber::GetThis();
        myhttp::IOManager* iom = myhttp::IOManager::GetThis();
        // iom->addTimer(usec / 1000, std::bind(&myhttp::IOManager::schedule, iom, fiber));
        iom->addTimer(usec / 1000, [iom, fiber](){
            iom->schedule(fiber);
        });
        myhttp::Fiber::YieldToHold();
        return 0;
    }

}
