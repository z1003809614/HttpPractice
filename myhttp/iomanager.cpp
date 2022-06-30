#include "iomanager.h"
#include "macro.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

namespace myhttp
{
    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event){
        switch (event)
        {
        case IOManager::READ:
            return read;
            break;
        case IOManager::WRITE:
            return write;
            break;
        default:
            MYHTTP_ASSERT2(false, "getContext");
            break;
        }
    }
    void IOManager::FdContext::resetContext(EventContext& ctx){
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }
    void IOManager::FdContext::triggerEvent(Event event){
        MYHTTP_ASSERT(events & event);
        events = (Event)(events & ~event);
        EventContext& ctx = getContext(event);
        if(ctx.cb){
            // 如果有回调，执行回调；
            ctx.scheduler->schedule(&ctx.cb);
        }
        else{
            // 没有回调，就执行指定的fiber；
            ctx.scheduler->schedule(&ctx.fiber);
        }
        ctx.scheduler = nullptr;
        return;
    }

    IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
        :Scheduler(threads, use_caller, name){
        // 创建epoll文件描述符
        m_epfd = epoll_create(5000);
        MYHTTP_ASSERT(m_epfd > 0);

        // 初始化管道
        int rt = pipe(m_tickleFds);
        MYHTTP_ASSERT(!rt);

        // 设置管道的读端添加epoll ET in事件
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_tickleFds[0];
        // 设置管道读端为非阻塞的；
        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        MYHTTP_ASSERT(!rt);
        // 将管道读端与设置好的event进行绑定，并添加到epoll内核事件表中；
        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        MYHTTP_ASSERT(!rt);

        contextResize(32);

        start();
    }
    IOManager::~IOManager(){
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        for(size_t i = 0; i < m_fdContexts.size(); ++i){
            if(m_fdContexts[i]){
                delete m_fdContexts[i];
            }
        }
    }

    void IOManager::contextResize(size_t size){
        m_fdContexts.resize(size);

        for(size_t i = 0; i < m_fdContexts.size(); ++i){
            if(!m_fdContexts[i]){
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }

    //1 success, 0 retry, -1 error
    int IOManager::addEvent(int fd, Event event, std::function<void()> cb){
        FdContext* fd_ctx = nullptr;
        RWMutexType::ReadLock lock(m_mutex);
        // 判断当前添加的任务是否在任务池内部；
        if( (int)m_fdContexts.size() > fd){
            fd_ctx = m_fdContexts[fd];
            lock.unlock();
        }
        else{
            lock.unlock();
            RWMutexType::WriteLock lock2(m_mutex);
            contextResize(fd * 1.5);
            fd_ctx = m_fdContexts[fd];
        }

        // 如果当前任务的事件和要添加的事件是一致的表明有多个线程同时操作了，
        // 或者说不应该出现这样的情况，进行断言处理；
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if(fd_ctx->events & event){
            MYHTTP_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd 
                    << " event=" << event
                    << " fd_ctx.event=" << fd_ctx->events;
            MYHTTP_ASSERT(!(fd_ctx->events& event));
        }
        
        // 根据当前的event的情况，决定后续向epoll内核添加事件的执行模式；
        int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events = EPOLLET | fd_ctx->events | event;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt){
            MYHTTP_LOG_ERROR(g_logger) << " epoll_ctl{" << m_epfd << ", "
                << op << "," << fd << "," << epevent.events << "}:"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return -1;
        }

        // 在上面执行成功后，改变当前任务的event的状态；
        ++m_pendingEventCount;
        fd_ctx->events = (Event)(fd_ctx->events | event);
        
        // 针对添加的具体事件，进行参数设置；
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        MYHTTP_ASSERT(!event_ctx.scheduler
                    && !event_ctx.fiber
                    && !event_ctx.cb);
        event_ctx.scheduler = Scheduler::GetThis();
        if(cb){
            event_ctx.cb.swap(cb);
        }else{
            event_ctx.fiber = Fiber::GetThis();
            MYHTTP_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
        }
        return 0;

    }
    bool IOManager::delEvent(int fd, Event event){
        RWMutexType::ReadLock lock(m_mutex);
        // 当前请求的fd不再任务池中，当然不能进行后续操作了；
        if((int)m_fdContexts.size() <= fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->mutex);
        // 当前要删除的事件没有注册，当然也不对；
        if(!(fd_ctx->events & event)){
            return false;
        }

        // 重新构建当前fd的事件；
        Event new_events = (Event) (fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt){
            MYHTTP_LOG_ERROR(g_logger) << " epoll_ctl{" << m_epfd << ", "
                << op << "," << fd << "," << epevent.events << "}:"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        // 重置当前抽象事件对应的具体事件参数；
        --m_pendingEventCount;
        fd_ctx->events = new_events;
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);
        return true;
    }
    bool IOManager::cancelEvent(int fd, Event event){
        RWMutexType::ReadLock lock(m_mutex);
        // 当前请求的fd不再任务池中，当然不能进行后续操作了；
        if( (int)m_fdContexts.size() <= fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->mutex);
        // 当前要删除的事件没有注册，当然也不对；
        if(!(fd_ctx->events & event)){
            return false;
        }

        // 重新构建当前fd的事件；
        Event new_events = (Event) (fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt){
            MYHTTP_LOG_ERROR(g_logger) << " epoll_ctl{" << m_epfd << ", "
                << op << "," << fd << "," << epevent.events << "}:"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        fd_ctx->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }

    bool IOManager::cancelAll(int fd){
        RWMutexType::ReadLock lock(m_mutex);
        // 当前请求的fd不再任务池中，当然不能进行后续操作了；
        if( (int)m_fdContexts.size() <= fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->mutex);
        // 当前要删除的事件没有注册，当然也不对；
        if(!(fd_ctx->events)){
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt){
            MYHTTP_LOG_ERROR(g_logger) << " epoll_ctl{" << m_epfd << ", "
                << op << "," << fd << "," << epevent.events << "}:"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        if(fd_ctx->events & READ){
            fd_ctx->triggerEvent(READ);
            --m_pendingEventCount;
        }
        if(fd_ctx->events & WRITE){
            fd_ctx->triggerEvent(WRITE);
            --m_pendingEventCount;
        }
        MYHTTP_ASSERT(fd_ctx->events == 0);
        return true;
    }

    // 这个地方父转子，可能出现问题；
    // 现在好像是可以正常使用，表明这个scheduler::getThis,可能实际指向的就是派生类的地址
    IOManager* IOManager::GetThis(){
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
    }

    void IOManager::tickle(){
        if(hasIdleThreads()){
            // MYHTTP_LOG_DEBUG(g_logger) << "tickle hasIdleThreads";
            return;
        }
        // MYHTTP_LOG_DEBUG(g_logger) << "tickle";
        int rt = write(m_tickleFds[1], "T", 1);
        MYHTTP_ASSERT(rt == 1);
    }
    
    bool IOManager::stopping() {
        uint64_t timeout = 0;
        return stopping(timeout);
    }
    
    bool IOManager::stopping(uint64_t& timeout){
        timeout = getNextTimer();
        return timeout == ~0ull
            && m_pendingEventCount == 0
            && Scheduler::stopping();
    }

    void IOManager::idle() {
        // 用来接收epoll内核中被触发的事件；
        epoll_event* events = new epoll_event[64]();
        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
            delete[] ptr;
        });

        while(true){
            uint64_t next_timeout = 0;
            if(stopping(next_timeout)){
                MYHTTP_LOG_INFO(g_logger) << " scheduler name=" << getName() 
                                          << " idle stopping exit";
                break;
            }

            // MYHTTP_LOG_DEBUG(g_logger) << " test idle next_timeout:" << next_timeout << " trans int :" << (int)next_timeout << " ~0ull: " << ~0ull ;

            int rt = 0;
            // 阻塞等待内核中的某些事件被触发；感觉不用写循环? 需要循环，某些导致的唤醒不需处理；
            do{
                static const int MAX_TIMEOUT = 3000;
                if(next_timeout != ~0ull){
                    next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
                }else{
                    next_timeout = MAX_TIMEOUT;
                }
                
                
                rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);

                if(rt < 0 && errno == EINTR){
                }else{
                    break;
                }
            }while(true);

           
            // 处理timer
            std::vector<std::function<void()> > cbs;
            listExpiredCb(cbs);
            MYHTTP_LOG_INFO(g_logger) << " cbs size:" << cbs.size();
            if(!cbs.empty()){
                schedule(cbs.begin(), cbs.end());
                cbs.clear();
            }

            // 处理被触发的事件
            for(int i = 0; i < rt; ++i){
                epoll_event& event = events[i];
                //如果是管道端的事件被触发的情况下；
                if(event.data.fd == m_tickleFds[0]){
                    uint8_t dummy;
                    while(read(m_tickleFds[0], &dummy, 1) == 1);
                    continue;
                }

                // 这里在对事件的属性进行处理，但不知道具体是为了什么；
                // 这里是在根据内核的事件属性，转换为自己定义的属性表达，如EPOLLIN->READ,EPOLLOUT->WRITE;
                FdContext* fd_ctx = (FdContext*) event.data.ptr;
                FdContext::MutexType::Lock lock(fd_ctx->mutex);

                if(event.events & (EPOLLERR | EPOLLHUP)){
                    event.events |= EPOLLIN | EPOLLOUT;
                }
                
                int real_events = NONE;
                if(event.events & EPOLLIN){
                    real_events |= READ; 
                }
                if(event.events & EPOLLOUT){
                    real_events |= WRITE; 
                }

                if((fd_ctx->events & real_events) == NONE){
                    continue;
                }

                int left_events = (fd_ctx->events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
                if(rt2){
                    MYHTTP_LOG_ERROR(g_logger) << " epoll_ctl{" << m_epfd << ", "
                        << op << "," << fd_ctx << "," << event.events << "}:"
                        << rt << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }

                // 调用触发函数，将事件本身的具体任务放到调度器中，等待执行；
                if(real_events & READ){
                    fd_ctx->triggerEvent(READ);
                    --m_pendingEventCount;
                }
                if(real_events & WRITE){
                    fd_ctx->triggerEvent(WRITE);
                    --m_pendingEventCount;
                }
            }
            // 该线程也中idle中切换到主协程中，去查看是否有该线程执行的任务；
            Fiber::ptr cur = Fiber::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();
            raw_ptr->swapOut();
        }
    }

    void IOManager::onTimerInsertedAtFront() {
        tickle();
    }

} // namespace myhttp



