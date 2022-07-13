#include "thread.h"
#include "log.h"
#include "util.h"
namespace myhttp{

    // 线程局部静态全局变量，记录创建该线程的对象地址；
    static thread_local Thread* t_thread = nullptr;
    // 这个信息其实用上面的指针就可完成，这里是为了方便使用；
    static thread_local std::string t_thread_name = "UNKNOW";

    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count){
        // 信号量初始化
        if(sem_init(&m_semaphore, 0, count)){
            throw std::logic_error("sem_init error");
        }
    }
    Semaphore::~Semaphore(){
        // 信号量销毁
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait(){
        // 请求信号量，如果无效，则阻塞
        if(sem_wait(&m_semaphore)){
            throw std::logic_error("sem_wait error");
        }
    }
    void Semaphore::notify(){
        // 释放信号量
        if(sem_post(&m_semaphore)){
            throw std::logic_error("sem_post error");
        }
    }

// ================================Thread=======================================
    
    Thread* Thread::GetThis(){
        return t_thread;
    }

    const std::string& Thread::GetName(){
        return t_thread_name;
    }

    void Thread::SetName(const std::string& name){
        if(t_thread){
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(std::function<void()> cb, const std::string& name)
        :m_cb(cb)
        ,m_name(name){
        if(name.empty()){
            m_name = "UNKNOW";
        }

        // 创建新线程的时候，将当前线程类的指针传递给内核生成的线程；
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if(rt){
            MYHTTP_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait();
    }
    Thread::~Thread(){
        // 当pthread_join执行失败的时候，需要使用pthread_detach来回收资源；
        if(m_thread){
            pthread_detach(m_thread);
        }
    }

    void Thread::join(){
        if(m_thread){
            int rt = pthread_join(m_thread, nullptr);
            if(rt){
                MYHTTP_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            // 如果这里不将m_thread置为0，会导致析构函数认为，对应的m_thread的线程未回收，从而调用pthread_detach再次回收，进而引发错误；
            m_thread = 0;
        }
    }

    void* Thread::run(void* arg){
        // 将传递过来的this指针，转换为Thread*类型指针；
        Thread* thread = (Thread*)arg;
        // 将自己封装的线程类的信息赋予内核线程；
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = myhttp::GetThreadId();

        pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());

        std::function<void()> cb;
        cb.swap(thread->m_cb); // 防止该函数内部存在智能指针，不会释放资源的情况，这样可以减少一层引用；

        thread->m_semaphore.notify();

        // 开始执行回调函数；
        cb();

        return 0;
    }
}