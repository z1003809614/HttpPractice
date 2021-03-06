# 线程和互斥量的封装和执行基本思路

## 各模块基本功能介绍

1. **Semaphore:** Posix标准的信号量封装
   1. 使用RAII(资源获取就是初始化)思想，封装了sem_init(),sem_destory()等api，对资源进行初始化和释放；
   2. 使用sem_wait()和sem_post()来申请和释放信号量；
2. **ScopedLockImpl,ReadScopedLockImpl,WriteScopedLockImpl** 对互斥量加锁解锁的模板类
   1. 依旧使用RAII的思想，初始化即加锁，析构即解锁
   2. 这些模板类其实意义不太大，内部调用了mutex的相关api，导致其通用性并不高；
3. **Mutex, RWMutex,Spinlock,CASLock** 不同类型的互斥量
   1. Mutex: pthread_mutex_t类型的互斥量
   2. RWMutex: pthread_rwlock_t类型的互斥量
   3. SpinLock: pthread_spinlock_t,自旋锁
   4. CASlock: std::atomic_flag, 原子锁
4. **Thread** 线程类，封装了线程的基本属性
   1. 这里使用了m_semaphore信号量机制，来保证一个线程真正的开始执行后，主线程才能够继续执行之后的任务；
   2. 这里是在构造函数内部使用成员变量m_seamaphore;
   3. 构造函数调用pthread_create();
   4. 析构函数调用pthread_detach;

#线程类使用方法
```
                                    自定义函数(call_back())
                                            |
                                            V
                        构造thread类，并将自定义函数传递给构造函数做为参数
                                            |
                                            V
                            主线程可调用Thread::join()，显示回收子线程
            或者不管子线程，等待其自动退出，对应的类析构的时候调用pthread_detach自动回收资源
```


## 知识点
1. **类成员变量初始化时机：**
   1. 在构造函数的函数体执行之前（即{}内的语句），所有的父类、虚类、non-static成员变量都已初始化完成。成员初始化列表（member initializer list）可用于父类、虚类、非静态成员变量的初始化（非默认的初始化，non-default initialization）。对于不能调用默认构造函数构造的基类、non-static成员变量，必须在初始化列表中进行初始化
   2. 必须显示初始化的情况
      1. reference member
      2. const member
      3. 带有参数的构造函数（基类 or 成员变量是有参构造函数的情况）
2. **Posix标准信号量和SystemV信号量标准的区别：** 
   > 简要的说，Posix是“可移植操作系统接口（Portable Operating System Interface ）的首字母简写，但它并不是一个单一的标准，而是一个电气与电子工程学会即IEEE开发的一系列标准，它还是由ISO（国际标准化组织）和IEC（国际电工委员会）采纳的国际标准。而System v是Unix操作系统众多版本的一个分支，它最初是由AT&T在1983年第一次发布，System v一共有四个版本，而最成功的是System V Release 4，或者称为SVR4。这样看来，一个是Unix 的标准之一（另一个标准是Open Group），一个是Unix众多版本的分支之一（其他的分支还有Linux跟BSD），应该来说，Posix标准正变得越来越流行，很多厂家开始采用这一标准
   1. POSIX信号量常用于线程；system v信号量常用于进程的同步。
   2. 从使用的角度，System V 信号量的使用比较复杂，而 POSIX 信号量使用起来相对简单
   3. 对 POSIX 来说，信号量是个非负整数。而 System V 信号量则是一个或多个信号量的集合，它对应的是一个信号量结构体，这个结构体是为 System V IPC 服务的，信号量只不过是它的一部分。
   4. Posix信号量是基于内存的，即信号量值是放在共享内存中的，它是由可能与文件系统中的路径名对应的名字来标识的。而System v信号量则是基于内核的，它放在内核里面。
   5. POSIX 信号量的头文件是 <semaphore.h>，而 System V 信号量的头文件是 <sys/sem.h>。
   6. Posix还有有名信号量，一般用于进程同步, 有名信号量是内核持续的
3. **互斥锁，自旋锁，原子锁** 
   1. 锁的宏观分类：
      1. 悲观锁（Pessimistic Lock）, 就是很悲观，每次去拿数据的时候都认为别人会修改。所以每次在拿数据的时候都会上锁。这样别人想拿数据就被挡住，直到悲观锁被释放。
      2. 乐观锁（Optimistic Lock）, 就是很乐观，每次去拿数据的时候都认为别人不会修改。所以**不会上锁，不会上锁**！但是如果想要更新数据，则会在更新前检查在读取至更新这段时间别人有没有修改过这个数据。如果修改过，则重新读取，再次尝试更新，循环上述步骤直到更新成功（当然也允许更新失败的线程放弃操作）。
         1. 什么是CAS呢？Compare-and-Swap，即比较并替换，也有叫做Compare-and-Set的，比较并设置。
            1. 比较：读取到了一个值A，在将其更新为B之前，检查原值是否仍为A（未被其他线程改动）
            2. 设置：如果是，将A更新为B，结束。[1]如果不是，则什么都不做。
         2. 上面的两步操作是原子性的，可以简单地理解为瞬间完成，在CPU看来就是一步操作。
         3. 缺点：
            1. CAS会导致“ABA问题”，若两个以上的线程读取到内存V中的值A,其中一个线程先进行更新操作,更新内存V中的值也为A，其它线程更新时，比对内存V中A时，发现一致，也进行了更新操作,但是此时对变量的操作已经不符合原来的期望了，这就是ABA问题。
            2. 只能保证一个变量的原子性(无法保证多个变量的互斥性)
      3. 乐观锁一般会使用**版本号机制(每次操作都增加版本号，可以解决ABA问题)**或**CAS算法实现**；
      4. 悲观锁阻塞事务，乐观锁回滚重试，它们各有优缺点，不要认为一种一定好于另一种。像乐观锁适用于写比较少的情况下，即冲突真的很少发生的时候，这样可以省去锁的开销，加大了系统的整个吞吐量。但如果经常产生冲突，上层应用会不断的进行重试，这样反倒是降低了性能，所以这种情况下用悲观锁就比较合适。
      5. **CAS乐观锁和自旋锁的疑问：** 
         1. 都是循环等待，有什么区别? 
            1. CAS乐观锁，等待一种情况的出现，即oldvalue == data,这个时候，就认为没有其他线程会修改这个变量，然后进行修改。但实际上，当前线程进行修改的时候，就一定没有其他线程进行修改了吗？没有锁的保护，显然是不一定的，但是通过上面的这样情况，可以极大概率的避免这样的错误情况的发生，故体现了其乐观的思想；**实际上，为了保证100%的成功，比较替代(cas)这样的两个步骤，在硬件上提供了原子性(一起完成)，其底层也是通过加锁来实现的，只不过粒度较小，可以看成是无锁；**
            2. 自旋锁，则是以循环等待这样的处理方式替代了互斥量的阻塞机制，其最终还是等待锁的释放，然后对自身进行加锁，再处理后续的情况。与互斥锁的本质一样，殊途同归，故是一种悲观锁；
            3. 感觉自旋锁和CAS乐观锁的区别在于，**锁变量本身就是数据那就是乐观锁；锁本身不是如数据，而是用来锁其他数据的，这个锁就成为了自旋锁**；
            4. atomic类本身就具有乐观锁的性质；
   2. 具体锁的概念介绍：
      1. 互斥锁(悲观)：用在执行长的代码块效率较高,如果只是执行一条少的指令,速度不如自旋锁和原子锁。互斥锁只有两种状态：锁住和未锁住
      2. 读写锁(悲观)：非常适合于对数据结构读的次数远远大于写的情况。同时可以有多个线程获得读锁，同时只允许有一个线程获得写锁。其他线程在等待锁的时候同样会进入睡眠。读写锁在互斥锁的基础上，允许多个线程“读”，在某些场景下能提高性能。
      3. 自旋锁(?感觉是悲观)：如果被锁住，其他线程获取锁就会空转等待，消耗CPU资源，不会去休眠。
      4. 原子操作：操作本身就具有原子性，不可拆分(就是底层提供了一个lock机制保证原子性)
         1. 本身不是锁的概念；
         2. 提供了一种类似互斥量的作用，通过其特性，进行加工可以实现自旋锁等功能；
   3. 互斥锁与自旋锁比较：
      1. 自旋锁是一种非阻塞锁，也就是说，如果某线程需要获取自旋锁，但该锁已经被其他线程占用时，该线程不会被挂起，而是在不断的消耗CPU的时间，不停的试图获取自旋锁。
      2. 互斥量是阻塞锁，当某线程无法获取互斥量时，该线程会被直接挂起，该线程不再消耗CPU时间，当其他线程释放互斥量后，操作系统会激活那个被挂起的线程，让其投入运行。
      3. 使用场景区分：
         1. 如果是多核处理器，如果预计线程等待锁的时间很短，短到比线程两次上下文切换时间要少的情况下，使用自旋锁是划算的。
         2. 如果是多核处理器，如果预计线程等待锁的时间较长，至少比两次线程上下文切换的时间要长，建议使用互斥量。
         3. 如果是单核处理器，一般建议不要使用自旋锁。因为，在同一时间只有一个线程是处在运行状态，那如果运行线程发现无法获取锁，只能等待解锁，但因为自身不挂起，所以那个获取到锁的线程没有办法进入运行状态，只能等到运行线程把操作系统分给它的时间片用完，才能有机会被调度。这种情况下使用自旋锁的代价很高。
         4. 如果加锁的代码经常被调用，但竞争情况很少发生时，应该优先考虑使用自旋锁，自旋锁的开销比较小，互斥量的开销较大。