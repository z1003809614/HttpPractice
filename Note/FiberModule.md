# Fiber协程模块封装思路

## 协程类重要函数设计思路

1. 使用两个线程局部的全局协程变量t_fiber和t_threadFiber,来完成协程任务的切换；
   1. t_fiber是裸指针，这样设计的目的是，t_fiber执行子协程，存在被执行完后进行释放的过程，如果使用智能指针，该全局变量一直持有该地址，导致无法释放；
   2. t_threadFiber是智能指针，且永远指向**主协程**，不会更改，随着线程的回收而回收，故不存在上述问题；
   3. 认真思考了一下，好像t_fiber也可以使用智能指针，因为当t_fiber会随着当前执行的协程不同而切换，当一个子协程执行完毕后，会切换到主协程中，完全不会存在不能够释放的情况；
2. 使用两个atomic量，来记录线程ID和线程总数，为了避免并发的影响所以需要使用原子量；
3. 协程核心属性
   1. 协程id;
   2. 协程运行栈的大小；
   3. 协程当前的运行状态；
   4. 协程的上下文；
   5. 协程要执行的函数；
4. 协程类必须要实现的行为：
   1. 协程需要实现“创建”（Create）、“启动” （Spawn）、“挂起” （Suspend）、“切换” （Swap）等操作
5. 使用ucontext实现了简单的协程封装； 



## [C++相关协程简介](https://blog.csdn.net/zzhongcy/article/details/89515037)

1. 协程概念简介
   1. 协程（Coroutines）是一种比线程更加轻量级的存在。协程完全由程序所控制（在用户态执行），带来的好处是性能大幅度的提升。一个操作系统中可以有多个进程；一个进程可以有多个线程；同理，一个线程可以有多个协程。协程是一个特殊的函数，这个函数可以在某个地方挂起，并且可以重新在挂起处继续运行。一个线程内的多个协程的运行是串行的，这点和多进程（多线程）在多核CPU上执行时是不同的。 多进程（多线程）在多核CPU上是可以并行的。当线程内的某一个协程运行时，其它协程必须挂起。
   2. 由于协程切换是在线程内完成的，涉及到的资源比较少。不像内核级线程（进程）切换那样，上下文的内容比较多，切换代价较大。协程本身是非常轻巧的，可以简单理解为只是切换了寄存器和协程栈的内容。这样代价就非常小
   3. 协程存在的问题：
      1. 协程切换的问题，实际上协程只有在等待IO的过程中才能重复利用线程，也就是说协程本质是通过多路复用来完成的。但是有个问题是，协程本身不是线程，只是一个特殊的函数，它不能被操作系统感知到（操作系统只能感知到进程和内核级线程），如果某个线程中的协程调用了阻塞IO，那么将会导致线程切换发生。因此只有协程是不够的，是无法解决问题的。还需要异步来配合协程。因此，实际上我们可以把协程可以看做是一种用户级线程的实现。协程+异步才能发挥出协程的最大作用
   4. 使用场景分析：
      1. 计算型的操作，利用协程来回切换执行，没有任何意义，来回切换并保存状态 反倒会降低性能。
      2. IO型的操作，利用协程在IO等待时间就去切换执行其他任务，当IO操作结束后再自动回调，那么就会大大节省资源并提供性能，从而实现异步编程（不等待任务结束就可以去执行其他代码）。
2. C++20协程语义(**无栈协程是普通函数的泛化**,从原理上来讲就无法在线程间流转：协程<->线程 多对多的情况)
   1. 关于协程（coroutine）是什么，在wikipedia[1]等资料中有很好的介绍，本文就不赘述了。在C++20中，协程的关键字只是语法糖：编译器会将函数执行的上下文（包括局部变量等）打包成一个对象，并让未执行完的函数先返回给调用者。之后，调用者使用这个对象，可以让函数从原来的“断点”处继续往下执行。
   2. 使用协程，编码时就不再需要费心费力的去把函数“切割”成多个小任务，只用按照习惯的流程写函数内部代码，并在允许暂时中断执行的地方加上co_yield语句，编译器就可以将该函数处理为可“分段执行”。
   3. 协程用起来的感觉有点像线程切换，因为函数的栈帧（stack frame）被编译器保存成了对象，可以随时恢复出来接着往下运行。但是实际执行时，协程其实还是单线程顺序运行的，并没有物理线程切换，一切都只是编译器的“魔法”。所以用协程可以完全避免多线程切换的性能开销以及资源占用，也不用担心数据竞争等问题。
   4. 可惜的是，C++20标准只提供了协程基础机制，并未提供真正实用的协程库（在C++23中可能会改善）。目前要用协程写实际业务的话，可以借助开源库，比如著名的cppcoro[2]。然而对于本文所述的场景，cppcoro也没有直接提供对应的工具（generator经过适当的包装可以解决这个问题，但是不太直观），因此我自己写了一个切割任务的协程工具类用于示例
   5. C++20 提供了三个新关键字(co_await、co_yield 和 co_return)，如果一个函数中存在这三个关键字之一，那么它就是一个协程。
3. GUN C的ucontext库(**有栈协程，能够实现协程与线程的多对多，但是相比无栈协程自然没有那么轻量**)
   1. ucontext机制是GNU C库提供的一组创建，保存，切换用户态执行上下文的API，从上面的描述可以看出ucontext_t结构体使得用户在程序中保存当前的上下文成为可能。我们也可以利用此实现用户级线程，即协程。
   2. 相关API：
      1. int makecontext(ucontext_t *ucp, void (*func)(), int argc, ...) 该函数用以初始化一个ucontext_t类型的结构，也就是我们所说的用户执行上下文。函数指针func指明了该context的入口函数，argc指明入口参数个数，该值是可变的，但每个参数类型都是int型，这些参数紧随argc传入。 另外，在调用makecontext之前，一般还需要显式的指明其初始栈信息（栈指针SP及栈大小）和运行时的信号屏蔽掩码（signal mask）。 同时也可以指定uc_link字段，这样在func函数返回后，就会切换到uc_link指向的context继续执行。
      2. int setcontext(const ucontext_t *ucp) 该函数用来将当前程序执行线索切换到参数ucp所指向的上下文状态，在执行正确的情况下，该函数直接切入到新的执行状态，不再会返回。比如我们用上面介绍的makecontext初始化了一个新的上下文，并将入口指向某函数entry()，那么setcontext成功后就会马上运行entry()函数。
      3. int getcontext(ucontext_t *ucp) 该函数用来将当前执行状态上下文保存到一个ucontext_t结构中，若后续调用setcontext或swapcontext恢复该状态，则程序会沿着getcontext调用点之后继续执行，看起来好像刚从getcontext函数返回一样。 这个操作的功能和setjmp所起的作用类似，都是保存执行状态以便后续恢复执行，但需要重点指出的是：getcontext函数的返回值仅能表示本次操作是否执行正确，而不能用来区分是直接从getcontext操作返回，还是由于setcontext/swapcontex恢复状态导致的返回，这点与setjmp是不一样的。
      4. int swapcontext(ucontext_t *oucp, ucontext_t *ucp) 理论上，有了上面的3个函数，就可以满足需要了（后面讲的libgo就只用了这3个函数，而实际只需setcontext/getcontext就足矣了），但由于getcontext不能区分返回状态，因此编写上下文切换的代码时就需要保存额外的信息来进行判断，显得比较麻烦。 为了简化切换操作的实现，ucontext 机制里提供了swapcontext这个函数，用来“原子”地完成旧状态的保存和切换到新状态的工作（当然，这并非真正的原子操作，在多线程情况下也会引入一些调度方面的问题，后面会详细介绍）。 
   3. 很容易通过上面的4个API及其组合加以实现，需要的工作仅在于设计一组数据结构保存暂不运行的context结构，提供一些调度的策略即可。 这方面的开源实现有很多，其中最著名的就是Go的前身，libtask库。
   4. **对于将“协程”映射到多OS线程执行的情形(该功能在schedule中实现了)**，就要稍稍复杂一些，但主要的问题是集中在共享任务队列的实现、调度线程间的互斥等，至于“协程”的映射问题，与单线程情况没有太大的区别。 对于这方面的开源借鉴，当然首推Go的运行时 —— 但由于标准Go实现没有使用GNU C库，而是自行设计了包括C编译器在内的整套工具链，因而就没有直接采用ucontext机制（尽管其内部实现机制与ucontext原理类似）。
4. 各大开源协程库如：libgo, libtask, libmill, boost, libco等，他们都属于stackfull协程，每个协程有完整的私有堆栈，里面的核心就是上下文切换(context)，而stackless的协程，比较出名的有protothreads，这个比较另类。
   1. 那么现有协程库，是怎么去实现context切换的呢，目前主要有以下几种方式：
      1. 使用ucontext系列接口，例如：libtask
      2. 使用setjmp/longjmp接口，例如：libmill
      3. 使用boost.context，纯汇编实现，内部实现机制跟ucontext完全不同，效率非常高，后面会细讲，tbox最后也是基于此实现
      4. 使用windows的GetThreadContext/SetThreadContext接口
      5. 使用windows的CreateFiber/ConvertThreadToFiber/SwitchToFiber接口