#include "timer.h"
#include "util.h"
#include "log.h"

namespace myhttp
{

    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");


    bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const{
        if(!lhs && !rhs){
            return false;
        }
        if(!lhs){
            return true;
        }
        if(!rhs){
            return false;
        }
        if(lhs->m_next < rhs->m_next){
            return true;
        }
        if(rhs->m_next < lhs->m_next){
            return false;
        }
        return lhs.get() < rhs.get();
    }

    Timer::Timer(uint64_t ms, std::function<void()> cb
                 ,bool recurring, TimerManager* manager)
        :m_recurring(recurring)
        ,m_ms(ms)
        ,m_cb(cb)
        ,m_manager(manager){
        m_next = myhttp::GetCurrentMS() + m_ms;
    }

    Timer::Timer(uint64_t next)
        :m_next(next){
    }


    bool Timer::cancel(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(m_cb){
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }

    bool Timer::refresh(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(!m_cb){
            return false;
        } 

        // 找到当前的这个定时器；
        auto it = m_manager->m_timers.find(shared_from_this());
        
        if(it == m_manager->m_timers.end()){
            return false;
        }
        
        // 先删除
        m_manager->m_timers.erase(it);
        
        // 重新设定该timer的下次执行时间
        m_next = myhttp::GetCurrentMS() + m_ms;
        
        // 然后再添加到manager中
        m_manager->m_timers.insert(shared_from_this()); 
        return true;
    }

    bool Timer::reset(uint64_t ms, bool from_now){
        
        if(ms == m_ms && !from_now){
            return true;
        }

        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);

        // 没有回调函数        
        if(!m_cb){
            return false;
        } 


        auto it = m_manager->m_timers.find(shared_from_this());
        
        if(it == m_manager->m_timers.end()){
            return false;
        }
        
        m_manager->m_timers.erase(it);
        
        uint64_t start = 0;
        
        if(from_now){
            start = myhttp::GetCurrentMS();
        }else{
            start = m_next - m_ms;
        }
        
        m_ms = ms;
        
        m_next = start + m_ms;
        
        m_manager->addTimer(shared_from_this(), lock);
        
        return true;
    }

// ======================================TimerManager===========================================
    TimerManager::TimerManager(){
        m_previouseTime = myhttp::GetCurrentMS();
    }

    TimerManager::~TimerManager(){

    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring){
        
        Timer::ptr timer(new Timer(ms, cb, recurring, this));
        RWMutexType::WriteLock lock(m_mutex);
        addTimer(timer, lock);
        return timer;
    }

    // 辅助条件定时器的执行
    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb){
        std::shared_ptr<void> tmp = weak_cond.lock();
        if(tmp){
            cb();
        }
    }

    // 添加条件定时器
    Timer::ptr TimerManager::addConditiaonTimer(uint64_t ms, std::function<void()> cb
                                            ,std::weak_ptr<void> weak_cond
                                            ,bool recurring)
    {
        return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
    }


    uint64_t TimerManager::getNextTimer(){
        RWMutexType::ReadLock lock(m_mutex);
        m_tickled = false;
        if(m_timers.empty()){
            return ~0ull;
        }

        // 获得定时器队列中的首元素
        const Timer::ptr& next = *m_timers.begin();
        uint64_t now_ms = myhttp::GetCurrentMS();
        if(now_ms >= next->m_next){
            return 0;
        }else{
            return next->m_next - now_ms;
        }
    }

    // 返回已经超过时间的定时器；
    void TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs){
        uint64_t now_ms = myhttp::GetCurrentMS();
        
        std::vector<Timer::ptr> expired;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if(m_timers.empty()){
                return;
            }
        }
        RWMutexType::WriteLock lock(m_mutex);

        bool rollover = detectClockRollover(now_ms);

        // 如果未检测到系统时间发生变化，并且首元素的下次执行时间还未到，证明没有需要执行的定时器；
        // PS：不知道为啥不使用getNextTimer()
        if(!rollover && ((*m_timers.begin())->m_next > now_ms)){
            return;
        }

        // MYHTTP_LOG_DEBUG(g_logger) << "rollover: " << rollover;

        Timer::ptr now_timer(new Timer(now_ms));
        
        // 如果系统时间变化，处理全部定时器，否则 只处理超过时间的定时器
        auto it = rollover ? m_timers.end() : m_timers.upper_bound(now_timer);

        // for(auto& t : m_timers){
        //     MYHTTP_LOG_DEBUG(g_logger) << "timer next time: " << (t)->m_next << " now_ms: " << now_ms;
        // }

        // while(it != m_timers.end() && (*it)->m_next == now_ms){
        //     ++it;
        // }

        // 把超时的定时器，添加到expired中；
        expired.insert(expired.begin(), m_timers.begin(), it);
        
        // 删除manager中的超时定时器；
        m_timers.erase(m_timers.begin(), it);
        
        cbs.reserve(expired.size());

        // 处理超时定时器，主要是处理某些需要定时执行的定时器；
        for(auto& timer : expired){
            cbs.push_back(timer->m_cb);
            if(timer->m_recurring){
                timer->m_next = now_ms + timer->m_ms;
                m_timers.insert(timer);
            }else{
                timer->m_cb = nullptr;
            }
        }
    }

    void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock){
        // 定时器链表中的首元素；
        auto it = m_timers.insert(val).first;
        
        // MYHTTP_LOG_DEBUG(g_logger) << "addtimer after m_times size:" << m_timers.size();
        
        // 判断该定时器是否为需要额外唤醒，进行处理；
        bool at_front = (it == m_timers.begin()) && !m_tickled;
        if(at_front){
            m_tickled = true;
        }
        lock.unlock();

        // 如果插入的定时器已经小于当前执行时间
        if(at_front){
            // MYHTTP_LOG_DEBUG(g_logger) << "come in addTimer at_front";
            onTimerInsertedAtFront();
        }
    }

    // 解决系统时间变化所产生的问题
    bool TimerManager::detectClockRollover(uint64_t now_ms){
        bool rollover = false;

        // MYHTTP_LOG_DEBUG(g_logger) << "now_ms: " << now_ms << " m_previousTime:" << m_previouseTime;

        if(now_ms < m_previouseTime 
            && now_ms < (m_previouseTime - 60 * 60 * 1000)){
            rollover = true;
        }
        m_previouseTime = now_ms;
        return rollover;
    }

    bool TimerManager::hasTimer(){
        RWMutexType::ReadLock lock(m_mutex);
        return !m_timers.empty();
    }

} // namespace myhttp
