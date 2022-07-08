#include <atomic>

#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
namespace myhttp{
    
    static Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    static std::atomic<uint64_t> s_fiber_id {0};
    static std::atomic<uint64_t> s_fiber_count {0};

    // 记录当前执行的协程；
    static thread_local Fiber* t_fiber = nullptr; 
    // 记录主协程的地址；
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024, "fiber stack size");

    class MallocStackAllocator{
        public:
            static void* Alloc(size_t size){
                return malloc(size);
            }

            static void Dealloc(void* vp, size_t size){
                return free(vp);
            }
    };

    using StackAllocator = MallocStackAllocator;
    
    uint64_t Fiber::GetFiberId(){
        if(t_fiber){
            return t_fiber->getId();
        }
        return 0;
    }

    Fiber::Fiber(){
        m_state = EXEC;
        SetThis(this);
        if(getcontext(&m_ctx)){
            MYHTTP_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count;
        MYHTTP_LOG_DEBUG(g_logger) << "Fiber::Fiber";
    }
    Fiber::Fiber(std::function<void()> cb, size_t stacksize)
        :m_id(++s_fiber_id)
        ,m_cb(cb){
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

        m_stack = StackAllocator::Alloc(m_stacksize);
        if(getcontext(&m_ctx)){
            MYHTTP_ASSERT2(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);

        MYHTTP_LOG_DEBUG(g_logger) << "Fiber::Fiber id" << m_id;
    }

    Fiber::~Fiber(){
        --s_fiber_count;
        if(m_stack){
            MYHTTP_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

            StackAllocator::Dealloc(m_stack, m_stacksize);
        } else{
            MYHTTP_ASSERT(!m_cb);
            MYHTTP_ASSERT(m_state == EXEC);

            Fiber* cur = t_fiber;
            if(cur == this){
                SetThis(nullptr);
            }
        }

        MYHTTP_LOG_DEBUG(g_logger) << "Fiber::~Fiber id" << m_id;
    }
    
    // 重置协程执行函数；
    void Fiber::reset(std::function<void()> cb){
        MYHTTP_ASSERT(m_stack);
        MYHTTP_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

        m_cb = cb;
        if(getcontext(&m_ctx)){
            MYHTTP_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
        
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    }
    // 切换到当前协程执行 
    void Fiber::swapIn(){
        SetThis(this);
        MYHTTP_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if(swapcontext(&(t_threadFiber->m_ctx), &m_ctx)){
            MYHTTP_ASSERT2(false, "swapInContext");
        }
    }
    // 切换到后台执行
    void Fiber::swapOut(){
        SetThis(t_threadFiber.get());

        if(swapcontext(&m_ctx, &(t_threadFiber->m_ctx))){
            MYHTTP_ASSERT2(false, "swapOutContext");
        }
    }

    void Fiber::SetThis(Fiber* f){
        t_fiber = f;
    }
    // 返回当前协程；
    Fiber::ptr Fiber::GetThis(){
        if(t_fiber){
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        MYHTTP_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }
    //协程切换到后台，并且设置为Ready状态
    void Fiber::YieldToReady(){
        Fiber::ptr cur = GetThis();
        cur->m_state = READY;
        cur->swapOut();
    }
    //协程切换到后台，并且设置为Hold状态
    void Fiber::YieldToHold(){
        Fiber::ptr cur = GetThis();
        cur->m_state = HOLD;
        cur->swapOut();
    }
    // 总协程数；
    uint64_t Fiber::ToTalFibers(){
        return s_fiber_count;
    }

    void Fiber::MainFunc(){
        Fiber::ptr cur = GetThis();
        MYHTTP_ASSERT(cur);
        // try{
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        // }catch(std::exception& ex){
        //     cur->m_state = EXCEPT;
        //     MYHTTP_LOG_ERROR(g_logger) << "Fiber Except:" << ex.what(); 
        // }catch(...){
        //     cur->m_state = EXCEPT;
        //     MYHTTP_LOG_ERROR(g_logger) << "Fiber Except:";
        // }

        // 本来uc_link就是执行调用协程的上一个协程的，可以用该变量完成协程的自动跳转；
        // 但是这里，使用了自己控制跳转的策略，为了能够正确析构，必须释放智能指针；
        // 所以下面提取了裸指针来完成上面的策略；
        auto raw_ptr = cur.get();
        // 这里的reset不是fiber的reset；
        cur.reset();
        raw_ptr->swapOut();

        MYHTTP_ASSERT2(false, "never reach");
    }
}