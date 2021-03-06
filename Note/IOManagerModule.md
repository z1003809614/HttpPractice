# IOManager调度器，基于Epoll事件触发机制

## 设计思路
1. 继承于scheduelr和timer(定时器)，并重写schedule::idle()，在该函数内部实现了epoll的控制流程；**这样就表明，一个调度器的所有子线程在空闲的时候都会陷入epoll_wait();**
2. 封装结构：
   1. epoll——fd
   2. m_tickleFds[2],用于生成管道，主要是服务于tickle()机制；
   3. 记录当前正在监听的事件数量m_pendingEventCount;
   4. 记录每个fd的事件上下文：包含的内容如下：
      1. 该fd包含哪些事件(read, write, none)
      2. read和write事件的具体执行的context:
         1. scheduler
         2. fiber : 这个其实是多余的和callbackFunc功能类似，而且不方便，用到的机会不大；
         3. callback func
3. 构造IOManager的时候就生成epoll管理器；并将其设置为ET模式(边沿触发，性能高)
4. 并使用池的思想，来存储fd信息；
5. 行为函数：
   1. IOManager::addEvent()
      1. 首先判断fd池能否容纳当前fd；
      2. 判断当前event是否已经添加过；
      3. 构造epoll_event结构，真正添加到epoll中；
      4. 修改自己封装的fdcontext中的events信息；
      5. 默认的回调函数，是当前协程；
6. 这里的基本思路是每一个事件，一旦触发，就默认将其删除掉，如果后续需要继续监听该事件，那么需要上层应用来重新添加该事件；


## 使用方法
```
                            调用者创建IOManager对象(自动启动调度器)
                                            |
                                            V
    使用IOManager::schedule向调度器添加调度任务 或者 addEvent()被动的等待事件触发即可
                                            |
                                            V
                        IOManager析构函数调用stop()，回收线程资源
```

## 存在问题
1. (也不能算问题，就是执行逻辑有点怪)主线程不执行epoll_wait,有可能提前析构iomanager对象，可能造成逻辑误判；
2. epoll_wait的设计，导致所有子线程都能够陷入，进而导致**惊群现象**；不过，目前没有实现tickle机制，也正因为这种现象，才实现了多线程并发的功能，但总归是不合理的；

## 知识点

1. 服务端编程需要构建高性能的IO模型，常见的IO模型主要有以下四种
   1. 同步阻塞IO
   2. 同步非阻塞IO 默认创建的socket都是阻塞的，非阻塞IO要求socket设置为NONBLOCK
   3. IO多路复用 经典Reactor设计模式，异步阻塞IO，select epoll
   4. 异步IO 异步非阻塞IO
2. 同步与异步 用户线程与内核的交互方式；同步是指用户发起IO请求后，需要等待或者轮询内核IO操作完成后才能继续执行；异步是指用户线程发起IO请求后继续执行，当内核操作完成后会通知线程或者调用用户线程注册的回调函数
3. 阻塞与非阻塞 用户线程调用内核IO操作的方式；阻塞是指IO操作需要彻底完成后才返回到用户空间，而非阻塞是指IO操作被调用后立即返回给用户一个状态值
4. (当前的设计存在这个问题)"惊群"简单地来讲，就是多个进程(线程)阻塞睡眠在某个系统调用上，在等待某个 fd(socket)的事件的到来。当这个 fd(socket)的事件发生的时候，这些睡眠的进程(线程)就会被同时唤醒，多个进程(线程)从阻塞的系统调用上返回，这就是"惊群"现象