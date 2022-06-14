#ifndef __MYHTTP_FIBER_H__
#define __MYHTTP_FIBER_H__

#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"

namespace myhttp{
    class Fiber : public std::enable_shared_from_this<Fiber> {
        public:
            typedef std::shared_ptr<Fiber> ptr;

            enum State{
                INIT,
                HOLD,
                EXEC,
                TERM,
                READY,
                EXCEPT
            };

        private:
            Fiber();

        public:
            Fiber(std::function<void()> cb, size_t stacksize = 0);
            ~Fiber();
            // 意图重复利用当前协程的内存信息，继续执行下一个func
            // 重置协程函数，并重置状态
            void reset(std::function<void()> cb);
            // 切换到当前协程执行 
            void swapIn();
            // 切换到后台执行
            void swapOut();

            uint64_t getId() const { return m_id ;}
        public:
            // 设置当前协程
            static void SetThis(Fiber* f);
            // 返回当前协程；
            static Fiber::ptr GetThis();
            //协程切换到后台，并且设置为Ready状态
            static void YieldToReady();
            //协程切换到后台，并且设置为Hold状态
            static void YieldToHold();
            // 总协程数；
            static uint64_t ToTalFibers();

            static void MainFunc();
            static uint64_t GetFiberId();

        private:
            uint64_t m_id = 0;
            uint32_t m_stacksize = 0;
            State m_state = INIT;

            ucontext_t m_ctx;
            void* m_stack = nullptr;

            std::function<void()> m_cb;
    };
}


#endif
