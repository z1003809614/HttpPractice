# Socket模块

## 模块设计
1. 封装属性
   - m_sock:socket 句柄
   - m_family: 协议族AF_INET,AF_INET6,UNIX
   - m_type: 类型TCP or UDP
   - m_protocol: 
   - m_isConnected: 当前socket是否连接
   - m_localAddress: 本地地址
   - m_remoteAddress: 远端地址
   - 隐藏属性：getsockopt，可以获得更多有关socket的信息，故提供了响应的方法；
2. getsockopt()/setsockopt()
   - sock：将要被设置或者获取选项的套接字。
   - level：选项所在的协议层。
   - optname：需要访问的选项名。
   - optval：对于getsockopt()，指向返回选项值的缓冲。对于setsockopt()，指向包含新选项值的缓冲。
   - optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。对于setsockopt()，现选项的长度。

## 使用方法
```
                             首先调用Address相关函数，获得地址信息addr
                                            |
                                            V
                                调用Socket，并传入addr获得sock
                                            |
                                            V
                                    调用sock->connect()
                                            |
                                            V
                        调用sock->recv接受信息，sock->send发送信息
                                            |
                                            V
                          sock析构自动调用系统函数close，释放系统资源
```