#include "fd_manager.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace myhttp
{

    FdCtx::FdCtx(int fd)
        :m_isInit(false)
        ,m_isSocket(false)
        ,m_sysNonblock(false)
        ,m_userNonblock(false)
        ,m_isClosed(false)
        ,m_fd(fd)
        ,m_recvTimeout(-1)
        ,m_sendTimeout(-1)
    {
        init();
    }
    FdCtx::~FdCtx(){
    }
    
    bool FdCtx::init(){
        if(m_isInit){
            return true;
        }
        m_recvTimeout = -1;
        m_sendTimeout = -1;

        // 读取当前fd的stat信息；如果读取失败则表示当前fd有问题，否则表示可以初始化；
        struct stat fd_stat;
        if( -1 == fstat(m_fd, &fd_stat)){
            m_isInit = false;
            m_isSocket = false;
        }else{
            m_isInit = true;
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }
        
        // 如果是socketfd，则将其设置为非阻塞的，并且hook；
        if(m_isSocket){
            int flags = fcntl_f(m_fd, F_GETFL, 0);
            if(!(flags & O_NONBLOCK)){
                fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
            }
            m_sysNonblock = true;
        }
        else{
            m_sysNonblock = false;
        }

        m_userNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }
    
    bool FdCtx::close(){
        return true;
    }

    void FdCtx::setTimeout(int type, uint64_t v){
        if(type == SO_RCVTIMEO){
            m_recvTimeout = v; 
        }
        else{
            m_sendTimeout = v;
        }
    }

    uint64_t FdCtx::getTimeout(int type){
        if(type == SO_RCVTIMEO){
            return m_recvTimeout; 
        }
        else{
            return m_sendTimeout;
        }
    }

    FdManager::FdManager(){
        m_datas.resize(64);
    }

    FdCtx::ptr FdManager::get(int fd, bool auto_create){
        // 之前没做fd为-1的校验，导致出错；
        if(fd == -1){
            return nullptr;
        }
        RWMutexType::ReadLock lock(m_mutex);
        if((int)m_datas.size() <= fd){
            // fd >= 文件描述符的池，且不自动创建，就返回空
            if(auto_create == false){
                return nullptr;
            }
        }
        else{
            // fd存在 返回该fd 或者 fd没有初始化，且不自动创建也返回，因为内部是nullptr
            if(m_datas[fd] || !auto_create){
                return m_datas[fd];
            }
        }
        lock.unlock();

        // 需要创建，且fd为空的情况
        RWMutexType::WriteLock lock2(m_mutex);
        FdCtx::ptr ctx(new FdCtx(fd));
        m_datas[fd] = ctx;
        return ctx;
    }
    
    void FdManager::del(int fd){
        RWMutexType::WriteLock lock(m_mutex);
        if((int)m_datas.size() <= fd){
            
        }
    }
} // namespace myhttp
