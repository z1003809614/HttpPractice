# Http协议解析模块

## 模块设计

1. 调用第三方Http_parser解析模块，进行解析，并将其封装为自己定义的http消息体类
2. HttpRequestParser类
   1. http_parser : 第三方定义的http_request解析类；
   2. HttpRequest::ptr m_data: 解析完成后封装为自己定义的消息体
   3. m_error: 解析错误码
3. HttpResponseParser类
   1. httpclient_parser： 第三方定义的httpclient_parser解析类
   2. HttpResponse::ptr : 解析完成后封装为自己定义的消息体
   3. m_error: 解析错误码

## 第三方解析过程
```
                    设置各个回调函数，用于配置第三方解析类
                                    |
                                    V
                   调用第三方http_parser的execute函数即可；
```