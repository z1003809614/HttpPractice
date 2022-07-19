# TCP服务器模块封装基于-》iomanager,address,socket

## 模块设计

1. TcpServer成员变量
   1. vector<scoket> : socket成员，即该服务器需要监听的socket集合
   2. IOManager* m_worker: 用于处理实际业务的调度器；
   3. IOManager* m_acceptWorker: 用于处理accept的调度器；
   4. recvTimeout : 接收超时时间
   5. name: 服务器名称
   6. isStop: 服务器是否停止
2. handleClient(socket)函数
   1. 提供一个接口，实现业务逻辑；

## 执行逻辑
```
                            初始化TcpServer对象
                                    |
                                    V
        提供监听地址addr,调用TCPServer::bind函数实现地址与socket的绑定和监听
                                    |
                                    V
            调用TcpServer::start()函数，实现内核accept，开始接受请求并处理业务
                                    |
                                    V
                    start()-> startAccept() -> handleClient()

```