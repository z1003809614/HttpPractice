# 链式定时器封装

## 定时器与定时管理器
1. 定时器class Timer的封装
   1. recurring：是否循环、 ms：执行周期、next:下次执行时间、cb:定时器触发后执行的任务、manager：定时管理器
   2. 提供cancel、refresh、reset、行为，由于timer被TimerManager所管理，所以这些方法中会通过TimerManager指针，去修改manager中的数据信息，耦合性很高；
2. 定时管理器class TimerManager的封装
   1. 定时器集合set<Timer> timers, tickled:标识是否有定时器触发， previousTimer:定时管理器上次处理定时任务的时间；
   2. 提供addTimer(添加定时器),getNextTimer(获得下一个定时器到当前的时间间隔),listExpiredCbs(返回所有超时定时器的回调函数，主要是方便IOManager的调度执行)；
   3. 
3. 关键函数，GetCurrentMS()，获取当前系统时间
   1. 该函数使用gettimeofday()来完成，该函数是符合posix标准的函数，即是C标准函数
4. IOManager继承了TimerManager
   1. 主要使用方法listExpiredCbs，来完成调度任务；

## Timer的执行逻辑
```
                                    调用者使用IOM->addTimer
                                            |
                                            V
                epoll_wait设置最短Timer的时间作为唤醒时间，等待Timer的时间来临
                                            |
                                            V
                    epoll_wait被唤醒后，会调用listExpiredCbs，获得回调任务
                                            |
                                            V
                    调用schedule将回调任务送入，IOManager的待执行序列中
                                            |
                                            V
                                    等待线程调度执行完成            
```


## 注意点
1. 该定时器任务，只能保证到了时间，我会触发执行，但是不能保证其延迟时间是0，延迟的时间和系统的调度有关系；