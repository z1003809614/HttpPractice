# 守护进程模块

## 模块设计

1. ProcessInfo结构体 -> 设置为单例模式
   1. 父进程Id
   2. 主进程id
   3. 父进程启动时间
   4. 主进程启动时间
   5. 主进程重启的次数
2. 全局函数start_daemon(argc,argv,main_cb,is_daemon);
   1. 根据is_daemon判断调用是否启动守护进程模式；
      1. is_daemon == true
         1. 调用real_daemon函数，内部使用fork()的逻辑，让子进程去执行real_start
         2. 父进程阻塞等待子进程的退出，如果是异常退出，则重新fork子进程，如果是正常退出，则父进程也正常退出；
      2. is_deamon == false
         1. 直接调用real_start函数