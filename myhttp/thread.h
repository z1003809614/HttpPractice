#ifndef __MYHTTP_THREAD_H__
#define __MYHTTP_THREAD_H__

#include <thread>
#include <functional>
#include <pthread.h>
#include <memory.h>

//pthread_xxx
//std::thread, pthread
namespace myhttp{

    class Thread{
        public:
            typedef std::shared_ptr<Thread> ptr;
            Thread(std::function<void()> cb, const std::string& name);
            ~Thread();

            pid_t getId() const {return m_id;}
            const std::string& getName() const {return m_name;}

            void join();
            static Thread* GetThis();
            static const std::string& GetName();
            static void SetName(const std::string& name);
        private:
            Thread(const Thread&) = delete;
            Thread(const Thread&&) = delete;
            Thread& operator=(const Thread&) = delete;

            static void* run(void* arg);
        private:
            pid_t m_id;
            pthread_t m_thread = 0;
            std::function<void()> m_cb;
            std::string m_name;
    };
}


#endif