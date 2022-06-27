#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace myhttp
{
    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    static thread_local Scheduler* t_scheduler = nullptr;
    // 当前线程正在执行的协程；
    // static thread_local Fiber* t_fiber = nullptr;

    // 这个use_caller就是表明是否使用主线程来做工作线程；
    // 这里我只实现了不使用主线程来做工作线程的版本，因此这个use_caller没用；
    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
        :m_name(name){
        
        MYHTTP_ASSERT(threads > 0);
        
        if(use_caller){
            // 生成当前线程的主协程；
            myhttp::Fiber::GetThis();
            --threads;
            MYHTTP_ASSERT(GetThis() == nullptr);
            //t_scheduler = this;

            // 当前调度器的根协程绑定run函数；
            // m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
            
            // 设定当前线程名称；
            myhttp::Thread::SetName(m_name);
            
            // 设定要执行的协程为根协程；
            // t_fiber = m_rootFiber.get();
            
            // 在调度器类内记录当前线程；
            m_rootThreadId = myhttp::GetThreadId();
            m_threadIds.push_back(m_rootThreadId);
        }else{
            m_rootThreadId = -1;
        }
        t_scheduler = this;
        m_threadCount = threads;
    }

    Scheduler::~Scheduler(){
        MYHTTP_ASSERT(m_stopping);
        if(GetThis() == this){
            t_scheduler = nullptr;
        }
    }

    Scheduler* Scheduler::GetThis(){
        return t_scheduler;
    }

    // Fiber* Scheduler::GetMainFiber(){
    //     return t_fiber;
    // }

    // 根据scheduler的线程相关配置，构建线程池；
    void Scheduler::start(){
        MutexType::Lock lock(m_mutex);
        if(!m_stopping){
            return;
        }
        m_stopping = false;
        MYHTTP_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);

        for(size_t i = 0; i < m_threadCount; ++i){
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                                , m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
    }
    
    void Scheduler::stop(){
        m_autoStop = true;
        // if(m_rootFiber && m_threadCount == 0 
        //    && (m_rootFiber->getState() == Fiber::TERM
        //         || m_rootFiber->getState() == Fiber::INIT))
        // {
        //     MYHTTP_LOG_INFO(g_logger) << this << " stopped";
        //     m_stopping = true;
        //     if(stopping()){
        //         return;
        //     }
        // }

        // bool exit_on_this_fiber = false;
        
        // if(m_rootThreadId != -1){
        //     MYHTTP_ASSERT(GetThis() == this);
        // }else{
        //     MYHTTP_ASSERT(GetThis() != this);
        // }

        m_stopping = true;
        for(size_t i = 0; i < m_threadCount; ++i){
            tickle();
        }

        // if(m_rootFiber){
        //     tickle();
        // }

        while(true){
            if(stopping()){
                std::vector<Thread::ptr> thrs;
                {
                    MutexType::Lock lock(m_mutex);
                    thrs.swap(m_threads);
                }
                for(auto& i : thrs){
                    i->join();
                }
                break;
            }
        }  
        // if(exit_on_this_fiber){
        // }

    }

    void Scheduler::setThis(){
        t_scheduler = this;
    }

    void Scheduler::run(){

        set_hook_enable(true);

        // 让每个线程的调度器都指向主线程调度器；
        setThis();
        // 初始化子线程的主协程；
        if(myhttp::GetThreadId() != m_rootThreadId){
           // t_fiber = Fiber::GetThis().get();
           Fiber::GetThis();
        }

        // 空闲协程；
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        // 非空闲协程；
        Fiber::ptr cb_fiber;
        // 协程与线程的混合体；
        FiberAndThread ft;
        
        while(true){
            //======================挑选当前线程需要处理的协程=========================
            ft.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                // 迭代当前的调度器已经存储的协程；
                auto it = m_fibers.begin();
                while(it != m_fibers.end()){
                    // 当前判断当前协程是否由当前线程执行；
                    if(it->thread != -1 && it->thread != myhttp::GetThreadId()){
                        ++it;
                        tickle_me = true;
                        continue;
                    }  
                    // 判断当前ft是有效的；
                    MYHTTP_ASSERT(it->fiber || it->cb);
                    // 如果当前ft是协程，并且正在执行则无需处理；
                    if(it->fiber && it->fiber->getState() == Fiber::EXEC){
                        ++it;
                        continue;
                    }
                    // 当前线程捕获当前ft，将其从调度器中删除；
                    ft = *it;
                    ++m_activeThreadCount;
                    m_fibers.erase(it);
                    break;
                }
            }

            // 是否需要通知其他线程；
            if(tickle_me){
                tickle();
            }

            //=====================开始对当前需要处理的协程进行分析执行=====================
            // 1. ft是 fiber的情况
            if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                            && ft.fiber->getState() != Fiber::EXCEPT)){
                // 调度器 活跃线程数加一；
                // ++m_activeThreadCount;
                // 开始执行协程任务；
                ft.fiber->swapIn();
                // 协程任务执行完成并将活跃线程数减一；
                --m_activeThreadCount;
                
                // 当前协程需要再次执行，则将其放入到调度器中；
                if(ft.fiber->getState() == Fiber::READY){
                    schedule(ft.fiber);
                }
                else if(ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT){
                    ft.fiber->m_state = Fiber::HOLD;// 挂起后的后续处理暂不知晓；
                }
                ft.reset();
            //2. ft是 回调函数 的情况；这里我们也将其封装为协程来执行；
            }else if(ft.cb){
                // cb_fiber什么时候不为空呢？ 
                // 当前线程是不断循环的，第二次需要处理协程的时候，就不为空；
                // 其实可以将cb_fiber的定义放入内部，这样做主要是避免对象的反复构建；
                if(cb_fiber){
                    cb_fiber->reset(ft.cb);
                }else{
                    cb_fiber.reset(new Fiber(ft.cb));
                    ft.cb = nullptr;
                }
                ft.reset();

                // ++m_activeThreadCount;
                cb_fiber->swapIn();
                --m_activeThreadCount;

                if(cb_fiber->getState() == Fiber::READY){
                    schedule(cb_fiber);
                    cb_fiber.reset();// 这里是智能指针应该没有析构问题；
                }else if(cb_fiber->getState() == Fiber::EXCEPT
                        || cb_fiber->getState() == Fiber::TERM){
                    cb_fiber->reset(nullptr);
                }else { //if(cb_fiber->getState() != Fiber::TERM){
                    cb_fiber->m_state = Fiber::HOLD; // 挂起操作；感觉没什么意义；
                    cb_fiber.reset(); // 这里应该会间接导致内部构建的Fiber析构；
                }
            }
            else{
                // 空闲fiber状态变为终止后，就退出循环，等于结束线程；
                if(idle_fiber->getState() == Fiber::TERM){
                    MYHTTP_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }

                // 将当前线程 判定为 空闲线程；
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                if(idle_fiber->getState() != Fiber::TERM
                        && idle_fiber->getState() != Fiber::EXCEPT){
                    idle_fiber->m_state = Fiber::HOLD; // ？
                }
                --m_idleThreadCount;
            }
        }
    }

    void Scheduler::tickle(){
        MYHTTP_LOG_INFO(g_logger) << " tickle ";
    }
    
    bool Scheduler::stopping(){
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }
    
    void Scheduler::idle(){
        
        while(!stopping()){
            MYHTTP_LOG_INFO(g_logger) << " idle ";
            sleep(1);
            Fiber::YieldToHold();
        }
        // Fiber::YieldToReady();
        // MYHTTP_LOG_INFO(g_logger) << " idle second comming!";
    }



} // namespace myhttp
