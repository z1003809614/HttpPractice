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
        if(rhs->m_next < rhs->m_next){
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
        auto it = m_manager->m_timers.find(shared_from_this());
        
        if(it == m_manager->m_timers.end()){
            return false;
        }
        m_manager->m_timers.erase(it);
        m_next = myhttp::GetCurrentMS() + m_ms;
        m_manager->m_timers.insert(shared_from_this()); 
        return true;
    }
    bool Timer::reset(uint64_t ms, bool from_now){
        if(ms == m_ms && !from_now){
            return true;
        }
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
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
        if(!rollover && ((*m_timers.begin())->m_next > now_ms)){
            return;
        }

        Timer::ptr now_timer(new Timer(now_ms));
        auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
        while(it != m_timers.end() && (*it)->m_next == now_ms){
            ++it;
        }
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.reserve(expired.size());

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

        if(at_front){
            // MYHTTP_LOG_DEBUG(g_logger) << "come in addTimer at_front";
            onTimerInsertedAtFront();
        }
    }

    // 解决系统时间变化所产生的问题
    bool TimerManager::detectClockRollover(uint64_t now_ms){
        bool rollover = false;
        if(now_ms < m_previouseTime &&
            now_ms < (m_previouseTime - 60 * 60 * 1000)){
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
