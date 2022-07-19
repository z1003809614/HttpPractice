# HttpSessionModule -> SocketStream -> Stream

## 模块设计

1. 核心功能：读取socketStream流数据，调用httpParser将其封装为Http响应体对象
2. 