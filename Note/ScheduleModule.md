# 调度器模型(核心封装了 线程-协程 的多对多调度)

## 调度器模块封装思路
1. 核心成员变量：
   1. 线程池 m_threads
   2. 待执行的协程队列 m_fibers
   3. 工作线程数量 m_activeThreadCount
   4. 空闲线程数量 m_idleThreadCount
2. 构造函数，原始设计为可选是否调用主线程来作工作线程，但是我没有实现该功能
3. Scheduler::start()函数
   1. 主要功能是初始化线程池，执行schedule::run函数；
4. Scheduler::stop()函数
   1. 遍历所有线程，将其唤醒，执行未完成的任务；
   2. 置换线程池到局部变量中，并循环等待回收所有子线程；
   3. 设置关键变量可以使得线程自动停止运行；
5. Scheduler::run()函数
   1. 初始化：
      1. 首先开启Hook功能，
      2. 调用Schedule::setThis()，使得每个线程都指向该调度器;
      3. 子线程调用Fiber::GetThis()，初始化每个子线程的协程配置；
   2. 定义空闲协程idle_fiber,与待执行fiber;
   3. 无限循环：
      1. 遍历调度器内部是否有当前线程能够调度的fiber or func;
      2. 执行fiber或 封装func到fiber，然后执行；
      3. 根据fiber执行完状态，进行后续处理，HOLD状态，调度器不做处理，应该丢给使用者来完成相关操作；
      4. 没有可执行的fiber,线程会执行idle_fiber,并利用idle_fiber的状态，来判断是否需要结束该线程；
   
## 调度器使用流程图
```
                                调用者创建Schedueler对象
                                            |
                                            V
                    使用Schedule::schedule(FiberOrFunc)向调度器添加调度任务
                                            |
                                            V
                        调用Schedule::Start()开启线程，并执行调度任务
                                            |
                                            V
                    调用Schedule::Stop()通知调度器在所有任务完成后可关闭
```

## 注意点
1. 调度器内的子线程共同指向同一个调度器，虽然t_schedule是线程全局变量；那为什么不将调度器设计为单例模式呢？ 因为，这样的设计可以实现，一个进程创建多个调度器，每个调度管理属于自己的线程来完成任务，实现了分组管理的模式；