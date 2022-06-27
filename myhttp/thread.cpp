#include "thread.h"
#include "log.h"
#include "util.h"
namespace myhttp{

    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count){
        if(sem_init(&m_semaphore, 0, count)){
            throw std::logic_error("sem_init error");
        }
    }
    Semaphore::~Semaphore(){
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait(){
        if(sem_wait(&m_semaphore)){
            throw std::logic_error("sem_wait error");
        }
    }
    void Semaphore::notify(){
        if(sem_post(&m_semaphore)){
            throw std::logic_error("sem_post error");
        }
    }

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
        // 将自己封装的线程类的信息赋予内核线程；
        Thread* thread = (Thread*)arg;
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