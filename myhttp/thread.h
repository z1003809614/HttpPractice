/**
 * @file thread.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief 线程与互斥量的封装
 * @version 0.1
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_THREAD_H__
#define __MYHTTP_THREAD_H__

#include "noncopyable.h"

#include <thread>
#include <functional>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>

//pthread_xxx
//std::thread, pthread
namespace myhttp{

    /**
     * @brief 信号量
     */
    class Semaphore : Noncopyable{
        public:
            /**
             * @brief 构造函数
             * @param[in] count 信号量值的大小
             */
            Semaphore(uint32_t count = 0);
            
            /**
             * @brief 析构函数
             */
            ~Semaphore();

            /**
             * @brief 获取信号量
             */
            void wait();

            /**
             * @brief 释放信号量
             */ 
            void notify();
        private:    
            sem_t m_semaphore;
    };

    /**
     * @brief 局部锁的模板实现
     */
    template<class T>
    struct ScopedLockImpl{
        public:
            
            /**
             * @brief 构造函数
             * @param[in] mutex Mutex
             */
            ScopedLockImpl(T& mutex)
                :m_mutex(mutex){
                    m_mutex.lock();
                    m_locked = true;
            }

            /**
             * @brief 析构函数,自动释放锁
             */
            ~ScopedLockImpl(){
                unlock();
            }
            
            /**
             * @brief 加锁
             */
            void lock(){
                if(!m_locked){
                    m_mutex.lock();
                    m_locked = true;
                }
            }

            /**
             * @brief 解锁
             */
            void unlock(){
                if(m_locked){
                    m_mutex.unlock();
                    m_locked = false;
                }
            }

        private:
            /// mutex
            T& m_mutex;
            /// 是否已上锁
            bool m_locked;
    };

    /**
     * @brief 局部读锁模板实现
     */
    template<class T>
    struct ReadScopedLockImpl{
        public:
            
            /**
             * @brief 构造函数
             * @param[in] mutex 读写锁
             */
            ReadScopedLockImpl(T& mutex)
                :m_mutex(mutex){
                    m_mutex.rdlock();
                    m_locked = true;
            }

            /**
             * @brief 析构函数,自动释放锁
             */
            ~ReadScopedLockImpl(){
                unlock();
            }

            /**
             * @brief 上读锁
             */
            void lock(){
                if(!m_locked){
                    m_mutex.rdlock();
                    m_locked = true;
                }
            }

            /**
             * @brief 释放锁
             */
            void unlock(){
                if(m_locked){
                    m_mutex.unlock();
                    m_locked = false;
                }
            }
        private:
            /// mutex
            T& m_mutex;
            /// 是否已上锁
            bool m_locked;
    };

    /**
     * @brief 局部写锁模板实现
     */
    template<class T>
    struct WriteScopedLockImpl{
        public:
            /**
             * @brief 构造函数
             * @param[in] mutex 读写锁
             */
            WriteScopedLockImpl(T& mutex)
                :m_mutex(mutex){
                    m_mutex.wrlock();
                    m_locked = true;
            }

            /**
             * @brief 析构函数
             */
            ~WriteScopedLockImpl(){
                unlock();
            }

            /**
             * @brief 上写锁
             */
            void lock(){
                if(!m_locked){
                    m_mutex.wrlock();
                    m_locked = true;
                }
            }

            /**
             * @brief 解锁
             */
            void unlock(){
                if(m_locked){
                    m_mutex.unlock();
                    m_locked = false;
                }
            }
        private:
            /// Mutex
            T& m_mutex;
            /// 是否已上锁
            bool m_locked;
    };

    /**
     * @brief 互斥量
     */
    class Mutex : Noncopyable{
        public:
            /// 局部锁
            typedef ScopedLockImpl<Mutex> Lock;
            
            /**
             * @brief 构造函数
             */
            Mutex(){
                pthread_mutex_init(&m_mutex, nullptr);
            }
            
            /**
             * @brief 析构函数
             */
            ~Mutex(){
                pthread_mutex_destroy(&m_mutex);
            }

            /**
             * @brief 加锁
             */
            void lock(){
                pthread_mutex_lock(&m_mutex);
            }

            /**
             * @brief 解锁
             */
            void unlock(){
                pthread_mutex_unlock(&m_mutex);
            }
        private:
            /// mutex
            pthread_mutex_t m_mutex;
    };

    /**
     * @brief 空锁(用于调试)
     */
    class NullMutex : Noncopyable{
        public:
            typedef ScopedLockImpl<NullMutex> Lock;
            NullMutex(){}
            ~NullMutex(){}
            void lock(){}
            void unlock(){}
    };
    
    /**
     * @brief 读写互斥量
     */
    class RWMutex : Noncopyable{
        public:
            /// 局部读锁
            typedef ReadScopedLockImpl<RWMutex> ReadLock;
            
            /// 局部写锁
            typedef WriteScopedLockImpl<RWMutex> WriteLock;
            
            /**
             * @brief 构造函数
             */
            RWMutex(){
                pthread_rwlock_init(&m_lock,nullptr);
            }
            
            /**
             * @brief 析构函数
             */
            ~RWMutex(){
                pthread_rwlock_destroy(&m_lock);
            }

            /**
             * @brief 上读锁
             */
            void rdlock(){
                pthread_rwlock_rdlock(&m_lock);
            }

            /**
             * @brief 上写锁
             */
            void wrlock(){
                pthread_rwlock_wrlock(&m_lock);
            }

            /**
             * @brief 解锁
             */
            void unlock(){
                pthread_rwlock_unlock(&m_lock);
            }
        private:
            /// 读写锁
            pthread_rwlock_t m_lock;
    };

    /**
     * @brief 空读写锁(用于调试)
     */
    class NullRWMutex : Noncopyable{
        public:
            typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
            typedef WriteScopedLockImpl<NullRWMutex> WriteLock;
            NullRWMutex(){}
            ~NullRWMutex(){}
            void rdlock(){}
            void wrlock(){}
            void unlock(){}
    };
    
    /**
     * @brief 自旋锁
     */
    class Spinlock : Noncopyable{
        public:
            /// 局部锁
            typedef ScopedLockImpl<Spinlock> Lock;

            /**
             * @brief 构造函数
             */
            Spinlock() {
                pthread_spin_init(&m_mutex, 0);
            }

            /**
             * @brief 析构函数
             */
            ~Spinlock() {
                pthread_spin_destroy(&m_mutex);
            }

            /**
             * @brief 上锁
             */
            void lock() {
                pthread_spin_lock(&m_mutex);
            }

            /**
             * @brief 解锁
             */
            void unlock() {
                pthread_spin_unlock(&m_mutex);
            }
        private:
            /// 自旋锁
            pthread_spinlock_t m_mutex;
    };

    /**
     * @brief 原子锁
     */
    class CASLock : Noncopyable {
        public:
            /// 局部锁
            typedef ScopedLockImpl<CASLock> Lock;

            /**
             * @brief 构造函数
             */
            CASLock() {
                m_mutex.clear();
            }

            /**
             * @brief 析构函数
             */
            ~CASLock() {
            }

            /**
             * @brief 上锁
             */
            void lock() {
                // std::memory_order_acquire ，表示在本线程中后续的读操作必须在本条原子操作完成后执行。
                // 因为不同的CPU可能实际的程序执行顺序并不是代码的顺序
                while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
            }

            /**
             * @brief 解锁
             */
            void unlock() {
                std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
            }
        private:
            /// 原子状态
            volatile std::atomic_flag m_mutex;
    };


    /**
     * @brief 线程类
     */
    class Thread{
        public: 
            /// 线程智能指针类型
            typedef std::shared_ptr<Thread> ptr;
            
            /**
             * @brief 构造函数
             * @param[in] cb 线程执行函数
             * @param[in] name 线程名称
             */
            Thread(std::function<void()> cb, const std::string& name);
            
            /**
             * @brief 析构函数
             */
            ~Thread();

            /**
             * @brief 线程ID
             */
            pid_t getId() const {return m_id;}
            
            /**
             * @brief 线程名称
             */
            const std::string& getName() const {return m_name;}

            /**
             * @brief 等待线程执行完成
             */
            void join();
            
            /**
             * @brief 获取当前的线程指针
             */
            static Thread* GetThis();
            
            /**
             * @brief 获取当前的线程名称
             */
            static const std::string& GetName();

            /**
             * @brief 设置当前线程名称
             * @param[in] name 线程名称
             * @bug 主线程通过 子线程对象 的指针调用该函数的时候，只会导致主线程的全局静态变量t_thread_name的值变化，而无法修改子线程的任何信息；
             */
            static void SetName(const std::string& name);
        private:

            /**
             * @brief 线程执行函数
             */
            static void* run(void* arg);
        private:
            /// 线程id
            pid_t m_id = -1;
            /// 线程结构
            pthread_t m_thread = 0;
            /// 线程执行函数
            std::function<void()> m_cb;
            /// 线程名称
            std::string m_name;
            /// 信号量
            Semaphore m_semaphore;
    };
}

#endif