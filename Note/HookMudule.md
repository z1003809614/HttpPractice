# Hook模块

## 设计思路
1. 重新定义与c库中一模一样的函数，来代替原有的函数的执行逻辑
2. 定义开关变量，提供是否使用hook的操作
3. 为了保证hook的函数能够提供原本c函数的功能，需要调用dlsym系统调用，实现对动态库的动态加载；
   1. dlsym函数的功能就是可以从共享库（动态库）中获取符号（全局变量与函数符号）地址，通常用于获取函数符号地址，这样可用于对共享库中函数的包装
   2. RTLD_DEFAULT表示按默认的顺序搜索共享库中符号symbol第一次出现的地址
   3. RTLD_NEXT表示在当前库以后按默认的顺序搜索共享库中符号symbol第一次出现的地址
4. 核心hook的思路是do_io函数
   1. 首先判断是否激活了hook,t_hook_enable == ture ?
   2. 然后查看fd是否存在
   3. 如果fd已经被关闭，则无需后续的处理
   4. 如果fd不是socket()或者用户设置未非阻塞，直接调用原本的c函数执行；
   5. 首先尝试调用原本的c函数操作，如果errno == EINTR,就重复执行
   6. 如果errno == EAGAIN,
      1. 首先根据fd的超时时间，添加一个超时定时器；
      2. 给fd添加事件，
      3. 事件添加成功后，换出当前协程（让出执行事件）;
      4. 当前协程被换入后进行后续判断并循环是否需要继续io操作；
5. 这里的do_io使用了addEvent()中不添加回调，默认使用当前协程来作为回调的特性；


## 知识点

1. extern C：在该模块内的函数，将使用c风格的编译后的签名，一般的系统调用，使用的都是c风格的函数；
2. 可变参数... ： c中使用...来表示可变参数
   1. 使用方法：
      1. va_list va：定义改变量用来配合可变参数的读取
      2. va_start(va, ...之前的那个变量)：一般是可变参数的个数
      3. (变量类型) v = va_arg(va, 变量类型)：读取参数到v中；
      4. va_end(va): 释放va资源；
   2. 注意
      1. 这一系列宏中，并不知道可变参数的个数和类型
3. c++11的可变参数：
   1. 同类型可变参数：
      1. initializer_list，传递同类型参数，类似vector，只不过使用的时候，就可以和普通传参一样；
   2. 不同类型可变参数：
      1. 函数模板
      - >//args是一个模板参数包；rest是一个函数参数包  
         //args表示零个或多个模板类型参数  
         //rest表示零个或多个函数参数  
         template<typename T, typename... args>  
         void foo(const T &t, const args&... rest); 
      2. 解包方法：
         1. 递归解包：
           1. 定义两个不同的函数，其中一个为终止函数/模板，另一个递归开始函数，每次递归调用，都会取得一个参数，进而进行解包
           2. 使用","语法来进行解包：先执行逗号之前的语句，然后返回逗号之后的数据
              1. >template <class ...Args>  
                  void expand(Args... args)  
                  {  
                    int arr[] = {(printarg(args), 0)...};  
                  }
      3. 类模板
      - >template< class... Types >  
         class tuple;
         std::tuple<int> tp1 = std::make_tuple(1);
         std::tuple<int, double> tp2 = std::make_tuple(1, 2.5);
         std::tuple<int, double, string> tp3 = std::make_tuple(1, 2.5, “”)  
       
4. 完美转发 std::forward()
   1. 函数内部具有名字的右值引用参数变量，是一个左值，这会导致一些move语言无法实现，进而产生了forward<>()完美转发的实现；
   2. 当我们将一个右值引用传入函数时，他在实参中有了命名，所以继续往下传或者调用其他函数时，根据C++ 标准的定义，这个参数变成了一个左值。那么他永远不会调用接下来函数的右值版本，这可能在一些情况下造成拷贝。为了解决这个问题 C++ 11引入了完美转发，根据右值判断的推倒，调用forward 传出的值，若原来是一个右值，那么他转出来就是一个右值，否则为一个左值。这样的处理就完美的转发了原有参数的左右值属性，不会造成一些不必要的拷贝


     