/**
 * @file fiber.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief 协程封装
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_FIBER_H__
#define __MYHTTP_FIBER_H__

#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"

namespace myhttp{

    class Scheduler;
    
    /**
     * @brief 协程类
     */
    class Fiber : public std::enable_shared_from_this<Fiber> {
    friend class Scheduler;
        public:
            typedef std::shared_ptr<Fiber> ptr;

            /**
             * @brief 协程状态
             */
            enum State {
                /// 初始化状态
                INIT,
                /// 暂停状态
                HOLD,
                /// 执行中状态
                EXEC,
                /// 结束状态
                TERM,
                /// 可执行状态
                READY,
                /// 异常状态
                EXCEPT
            };

        private:
            /**
             * @brief 无参构造函数
             * @attention 每个线程第一个协程的构造
             */
            Fiber();

        public:
            
            /**
             * @brief 构造函数
             * @param[in] cb 协程执行的函数
             * @param[in] stacksize 协程栈大小
             * 我这里没有实现该功能 use_caller 是否在MainFiber上调度
             */
            Fiber(std::function<void()> cb, size_t stacksize = 0);
            
            /**
             * @brief 析构函数
             */
            ~Fiber();
            
            /**
             * @brief 重置协程执行函数,并设置状态
             * @pre getState() 为 INIT, TERM, EXCEPT
             * @post getState() = INIT
             * 意图重复利用当前协程的内存信息，继续执行下一个func
             * 重置协程函数，并重置状态
             */
            void reset(std::function<void()> cb);
            
            /**
             * @brief 将当前协程切换到运行状态
             * @pre getState() != EXEC
             * @post getState() = EXEC
             */
            void swapIn();
            
            /**
             * @brief 将当前协程切换到后台
             */
            void swapOut();

            /**
             * @brief 返回协程id
             */
            uint64_t getId() const { return m_id ;}

            /**
             * @brief 返回协程状态
             */
            State getState() const {return m_state;}

        public:
            /**
             * @brief 设置当前线程的运行协程
             * @param[in] f 运行协程
             */
            static void SetThis(Fiber* f);
            
            /**
             * @brief 返回当前所在的协程
             */
            static Fiber::ptr GetThis();
            
            /**
             * @brief 将当前协程切换到后台,并设置为READY状态
             * @post getState() = READY
             */
            static void YieldToReady();
            
            /**
             * @brief 将当前协程切换到后台,并设置为HOLD状态
             * @post getState() = HOLD
             */
            static void YieldToHold();
            
            /**
             * @brief 返回当前协程的总数量
             */
            static uint64_t ToTalFibers();

            /**
             * @brief 协程执行函数
             * @post 执行完成返回到线程主协程
             */
            static void MainFunc();
            
            /**
             * @brief 获取当前协程的id
             */
            static uint64_t GetFiberId();

        private:
            /// 协程id
            uint64_t m_id = 0;
            
            /// 协程运行栈大小
            uint32_t m_stacksize = 0;
            
            /// 协程状态
            State m_state = INIT;
           
            /// 协程上下文
            ucontext_t m_ctx;
            
            /// 协程运行栈指针
            void* m_stack = nullptr;
            
            /// 协程运行函数
            std::function<void()> m_cb;
    };
}


#endif
