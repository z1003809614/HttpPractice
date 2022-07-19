# HttpConnection模块封装

## 模块设计
1. 模块简介
   1. Uri模块
      1. 将url解析为各个字段属性，将其封装起来；
   2. HttpConnection类
      1. 该模块类似HttpSession类，但角度不同，该类服务于请求发送者
      2. DoRequest处理流程：
         1. 通过Uri获得address
         2. 根据addr生成socket
         3. 封装HttpRequets响应体
         4. 调用SocketStream的函数将其发送(这里处理方式比较奇怪，初始化了一个带有Sock的HttpConnection，然后调用sendRequest进行发送)
   3. HttpConnectionPool类
      1. 有自己的host，默认内部所有connection向自己的pool的host发送数据，而不向request的host发送数据；
      2. 重新实现了dorequest的逻辑，并没有应用HttpConnection的addr;
      3. 这里的Url其实只是Path

## 发送流程
```
               HttpConnection                        HttpConnectionPool
                     |                                       |
                     V                                       V      
               根据Uri获得Addr       从连接池中获得一个Httpconnection(一定含有socket)
                     |                                       |
                     V                                       V
             根据Addr绑定Socket               直接使用该HttpConnection发送数据
                     |
                     V
      使用Socket生成HttpConnection对象
                     |
                     V
         使用该HttpConnection发送数据     
```