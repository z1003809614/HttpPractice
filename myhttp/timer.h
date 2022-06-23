#ifndef __MYHTTP_TIMER_H__
#define __MYHTTP_TIMER_H__

#include <memory>
#include <functional>
#include <set>
#include "thread.h"

namespace myhttp{

    class TimerManager;
    class Timer : public std::enable_shared_from_this<Timer> {
    friend class TimerManager;
        public:
            typedef std::shared_ptr<Timer> ptr;
        private:
            Timer(uint64_t ms, std::function<void()> cb,
                 bool recurring, TimerManager* manager);

            Timer(uint64_t next);

            bool cancel();
            bool refresh();
            bool reset(uint64_t ms, bool from_now);
        
        private:
            bool m_recurring = false;       //是否循环定时器
            uint64_t m_ms = 0;              //执行周期
            uint64_t m_next = 0;            //精确的执行时间
            std::function<void()> m_cb;
            TimerManager* m_manager = nullptr;

        private:
            struct Comparator
            {
                bool operator() (const Timer::ptr& lhs, const Timer::ptr& rhs) const;
            };
            
    };


    class TimerManager{
    friend class Timer;
        public:
            typedef RWMutex RWMutexType;

            TimerManager();
            virtual ~TimerManager();

            Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                                ,bool recurring = false);
            Timer::ptr addConditiaonTimer(uint64_t ms, std::function<void()> cb
                                          ,bool recurring = false);

            uint64_t getNextTimer();

            void listExpiredCb(std::vector<std::function<void()> >& cbs);

        protected:
            virtual void onTimerInsertedAtFront() = 0;
            void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

        private:
            // 用来检测服务器时间变化的问题
            bool detectClockRollover(uint64_t now_ms);

        private:
            RWMutexType m_mutex;
            // 链式定时器存储方式
            std::set<Timer::ptr, Timer::Comparator> m_timers;
            // 标记是否被提前唤醒；
            bool m_tickled = false;
            // 记录当前定时器管理器的创立时间，用于后续判断是否改动过系统时间
            uint64_t m_previouseTime = 0;
    };

}


#endif