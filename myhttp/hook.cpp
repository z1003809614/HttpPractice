#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "fd_manager.h"

#include <dlfcn.h>
namespace myhttp
{
    myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");
    static thread_local bool t_hook_enable = false;

    #define HOOK_FUN(XX) \
        XX(sleep) \
        XX(usleep) \
        XX(nanosleep) \
        XX(socket) \
        XX(connect) \
        XX(accept) \
        XX(read) \
        XX(readv) \
        XX(recv) \
        XX(recvfrom) \
        XX(recvmsg) \
        XX(write) \
        XX(writev) \
        XX(send) \
        XX(sendto) \
        XX(close) \
        XX(fcntl) \
        XX(ioctl) \
        XX(getsockopt) \
        XX(setsockopt)


    void hook_init(){
        static bool is_inited = false;
        if(is_inited){
            return;
        }
        // 从系统调用的动态库中找到对应的函数签名的地址，并赋予name_f；
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

    struct timer_info
    {
        int cancelled = 0;
    };

    template<typename OriginFun, typename ... Args>
    static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
            uint32_t event, int timeout_so, Args&&... args)
    {
        // 如果不hook就按原来函数执行；
        if(!myhttp::t_hook_enable){
            return fun(fd, std::forward<Args>(args)...);
        }

        // 没找到fd的情况；
        myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
        if(!ctx){
            return fun(fd, std::forward<Args>(args)...);
        }

        // fd已经被close的情况
        if(ctx->isClose()){
            errno = EBADF;
            return -1;
        }

        // fd不是socket的情况 或者 用户设置为非阻塞的情况；
        if(!ctx->isSocket() || ctx->getUserNonblock()){
            return fun(fd, std::forward<Args>(args)...);
        }
        
        uint64_t to = ctx->getTimeout(timeout_so);          // 获得超时时间
        std::shared_ptr<timer_info> tinfo(new timer_info);  // timer条件

    // 自定义io操作（hook）
    retry:
        // 首先尝试用原来的函数进行io操作；
        ssize_t n = fun(fd, std::forward<Args>(args)...);
        // 如果没有读取到数据，就尝试多次读取；
        while(n == -1 && errno == EINTR){
            n = fun(fd, std::forward<Args>(args)...);
        }
        // 当errno类型为EAGAIN的时候进行相应的操作；
        if(n == -1 && errno == EAGAIN){
            myhttp::IOManager* iom = myhttp::IOManager::GetThis();
            myhttp::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo); 

            // 添加条件定时任务（用于判断当前io操作是否已经执行）
            // 这里fd的事件是在用到的时候添加，用完就删除对应的事件，下次用再添加；
            if(to != (uint64_t)-1){
                timer = iom->addConditiaonTimer(to, [winfo, fd, iom, event](){
                    auto t = winfo.lock();
                    if(!t || t->cancelled){
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (myhttp::IOManager::Event)(event));
                }, winfo);
            }

            // 添加事件
            int rt = iom->addEvent(fd, (myhttp::IOManager::Event)(event));
            if(rt){ // 事件添加失败的情况
                MYHTTP_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                    << fd << ", " << event << ")";
                if(timer){
                    timer->cancel();
                }
                return -1;
            }else{ // 事件添加成功的情况
                // 换出当前fiber，等待event出现；
                myhttp::Fiber::YieldToHold();
                // fiber被唤醒的后执行的任务；
                if(timer){
                    timer->cancel();
                }
                if(tinfo->cancelled){
                    errno = tinfo->cancelled;
                    return -1;
                }

                goto retry;
            }
        }

        return n;
    }
} // namespace myhttp

extern "C"{
    //这里的宏，主要是对sleep_f，usleep_f进行定义和初始化；
    #define XX(name) name ## _fun name ## _f = nullptr;
        HOOK_FUN(XX)
    #undef XX

    // 定义sleep和usleep的同名函数
    unsigned int sleep(unsigned int seconds){
        if(!myhttp::t_hook_enable) {
            // 当该线程不进行hook的时候，执行系统调用中的对应函数；
            return sleep_f(seconds);
        }

        // 将原本是需要睡眠的线程，自定义其sleep行为，将其任务添加到定时任务中；
        // 这样做可以保证线程最大化的应用，否则当前线程会因为某个协程需要sleep，而导致整个线程不工作；
        myhttp::Fiber::ptr fiber = myhttp::Fiber::GetThis();
        myhttp::IOManager* iom = myhttp::IOManager::GetThis();
        iom->addTimer(seconds * 1000, std::bind((void(myhttp::Scheduler::*)
                (myhttp::Fiber::ptr, int thread))&myhttp::IOManager::schedule
                , iom, fiber, -1));
        // iom->addTimer(seconds * 1000, [iom, fiber](){
        //     iom->schedule(fiber);
        // });

        myhttp::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec){
        if(!myhttp::t_hook_enable){
            return usleep_f(usec);
        }
        myhttp::Fiber::ptr fiber = myhttp::Fiber::GetThis();
        myhttp::IOManager* iom = myhttp::IOManager::GetThis();
        iom->addTimer(usec / 1000, std::bind((void(myhttp::Scheduler::*)
                (myhttp::Fiber::ptr, int thread))&myhttp::IOManager::schedule
                , iom, fiber, -1));
        // iom->addTimer(usec / 1000, [iom, fiber](){
        //     iom->schedule(fiber);
        // });
        myhttp::Fiber::YieldToHold();
        return 0;
    }

    // 纳秒级的sleep函数；
    int nanosleep(const struct timespec *req, struct timespec *rem){
        if(!myhttp::t_hook_enable){
            return nanosleep_f(req, rem);
        }

        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        myhttp::Fiber::ptr fiber = myhttp::Fiber::GetThis();
        myhttp::IOManager* iom = myhttp::IOManager::GetThis();

        iom->addTimer(timeout_ms, std::bind((void(myhttp::Scheduler::*)
                (myhttp::Fiber::ptr, int thread))&myhttp::IOManager::schedule
                , iom, fiber, -1));

        myhttp::Fiber::YieldToHold();
        return 0;
    }

    int socket(int domain, int type, int protocol){
        if(!myhttp::t_hook_enable){
            return socket_f(domain, type, protocol);
        }
        int fd = socket_f(domain, type, protocol);
        if(fd == -1){
            return fd;
        }
        myhttp::FdMgr::GetInstance()->get(fd,true);
        return fd;
    }
    
    int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms){
        if(!myhttp::t_hook_enable){
            return connect_f(fd, addr, addrlen);
        }
        myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
        if(!ctx || ctx->isClose()){
            errno = EBADF;
            return -1;
        }

        if(!ctx->isSocket()){
            return connect_f(fd, addr, addrlen);
        }

        if(ctx->getUserNonblock()){
            return connect_f(fd, addr, addrlen);
        }

        int n = connect_f(fd, addr, addrlen);
        if(n == 0){
            return 0;
        }else if(n != -1 || errno != EINPROGRESS){
            return n;
        }

        myhttp::IOManager* iom = myhttp::IOManager::GetThis();
        myhttp::Timer::ptr timer;
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);

        if(timeout_ms != (uint64_t)-1){
            timer = iom->addConditiaonTimer(timeout_ms, [winfo, fd, iom](){
                auto t = winfo.lock();
                if(!t || ->cancelled){
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, myhttp::IOManager::WRITE);
            }, winfo);
        }

        int rt = iom->addEvent(fd, myhttp::IOManager::WRITE);
        if(rt == 0){
            myhttp::Fiber::YieldToHold();
            if(timer){
                timer->cancel();
            }
            if(tinfo->cancelled){
                errno = tinfo->cancelled;
                return -1;
            }
        }else{
            if(timer){
                timer->cancel();
            }
            MYHTTP_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
        }
        int error = 0;
        socklen_t len = sizeof(int);
        if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)){
            return -1;
        }
        if(!error){
            return 0;
        }else{
            errno = error;
            return -1;
        }
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
        return connect_f(sockfd, addr, addrlen);
    }

    int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
        int fd = do_io(s, accept_f, "accept", myhttp::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
        if(fd >= 0){
            myhttp::FdMgr::GetInstance()->get(fd, true);
        }
        return fd;
    }


    ssize_t read(int fd, void *buf, size_t count){
        return do_io(fd, read_f, "read", myhttp::IOManager::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
        return do_io(fd, readv_f, "readv", myhttp::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags){
        return do_io(sockfd, recv_f, "recv", myhttp::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen)
    {
        return do_io(sockfd, recvfrom_f, "recvfrom", myhttp::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){
        return do_io(sockfd, recvmsg_f, "recvmsg", myhttp::IOManager::READ, SO_RCVTIMEO, msg, flags);
    }


    size_t write(int fd, const void *buf, size_t count){
        return do_io(fd, write_f, "write", myhttp::IOManager::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt){
        return do_io(fd, writev_f, "writev", myhttp::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int sockfd, const void *buf, size_t len, int flags){
        return do_io(sockfd, send_f, "send", myhttp::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);
    }

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                    const struct sockaddr *dest_addr, socklen_t addrlen)
    {
        return do_io(sockfd, sendto_f, "sendto", myhttp::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags, dest_addr, addrlen);
    }

    ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags){
        return do_io(sockfd, sendmsg_f, "sendmsg", myhttp::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
    }

    int close(int fd){
        if(!myhttp::t_hook_enable){
            return close_f(fd);
        }

        myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
        if(ctx){
            auto iom = myhttp::IOManager::GetThis();
            if(iom){
                iom->cancelAll(fd);
            }
            myhttp::FdMgr::GetInstance()->del(fd);
        }
        return close_f(fd);
    }


    int fcntl(int fd, int cmd, .../*arg*/){
        if(!myhttp::t_hook_enable){
            return fcntl_f(fd, cmd, .../*arg*/);
        }

        va_list va;
        va_start(va, cmd);
        switch (cmd)
        {
            case F_SETFL:
                {
                    int arg = va_arg(va,int);
                    va_end(va);
                    myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
                    if(!ctx || ctx->isClose() || !ctx->isSocket()){
                        return fcntl_f(fd, cmd, arg);
                    }
                    ctx->setUserNonblock(arg & O_NONBLOCK);
                    if(ctx->getSysNonblock()){
                        arg |= O_NONBLOCK;
                    }else{
                        arg &= ~O_NONBLOCK;
                    }
                    return fcntl_f(fd, cmd, arg);
                }
                break;
            case F_GETFL:
                {
                    va_end(va);
                    int arg = fcntl_f(fd, cmd);
                    myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
                    if(!ctx || ctx->isClose() || !ctx->isSocket()){
                        return arg;
                    }
                    if(ctx->getUserNonblock()){
                        return arg | O_NONBLOCK;
                    }else{
                        return arg & ~O_NONBLOCK;
                    }
                }
                break;
            case F_DUPFD:
            case F_DUPFD_CLOEXEC:
            case F_SETFD:
            case F_SETOWN:
            case F_SETSIG:
            case F_SETLEASE:
            case F_NOTIFY:
            case F_SETPIPE_SZ:
                {
                    int arg = va_arg(va, int);
                    va_end(va);
                    return fcntl_f(fd, cmd,  arg);
                }
                break;
            case F_GETFD:
            case F_GETSIG:
            case F_GETOWN:
            case F_GETLEASE:
            case F_GETPIPE_SZ:
                {
                    va_end(va);
                    return fcntl_f(fd, cmd);
                }
                break;
            case F_SETLK:
            case F_SETLKW:
            case F_GETLK:
                {
                    struct flock* arg = va_arg(va, struct flock*);
                    va_end(va);
                    return fcntl_f(fd, cmd, arg);
                }
                break;
            case F_GETOWN_EX:
            case F_SETOWN_EX:
                {
                    struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                    va_end(va);
                    return fcntl_f(fd, cmd, arg);
                }
                break;
            default:
                va_end(va);
                return fcntl_f(fd, cmd);
        }
    }

    int ioctl(int fd, unsigned long request, ...){
        va_list va;
        va_start(va, request);
        void* arg = va_arg(va, void*);
        va_end(va);

        if(FIONBIO == request){
            bool user_nonblock = !!*(int*)arg;
            myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()){
                return ioctl_f(fd, request, arg);
            }
            ctx->setUserNonblock(user_nonblock);
        }
        return ioctl_f(fd, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname,
                    void *optval, socklen_t *optlen)
    {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }

    int setsockopt(int sockfd, int level, int optname,
                    const void *optval, socklen_t optlen)
    {
        if(!myhttp::t_hook_enable){
            return setsockopt(sockfd, level, optname, optval, optlen);
        }
        if(level == SOL_SOCKET){
            if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO){
                myhttp::FdCtx::ptr ctx = myhttp::FdMgr::GetInstance()->get(sockfd);
                if(ctx){
                    const timeval* v = (const timeval*) optval;
                    ctx->setTimeout(optname, tv->tv_sec * 1000 + tv->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }

}
