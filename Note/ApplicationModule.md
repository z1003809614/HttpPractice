# Application应用启动封装


## 模块设计

1. 使用env处理启动参数，并根据配置文件加载服务器配置信息，调用守护进程模块，进行服务器配置并启动；


## 执行逻辑
```
                调用Application::Init()->初始化Env实例->判断当前pid是否正在使用->创建工作目录
                                                |
                                                V
            根据Env获取配置参数，调用Application::run函数，调用deamon()->调用Application::main()
                                                |
                                                V
                        在工作路径中写入当前进程pid号，调用Application::run_fiber()
                                                |
                                                V
                            run_fiber()中， 根据配置信息，启动Http服务器(1~n个)               
```