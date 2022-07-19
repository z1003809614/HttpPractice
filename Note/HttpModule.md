# Http响应体封装

## 模块设计

1. HttpRequest响应体
   1. 基本属性
      1. 请求方法：method
      2. http协议版本： version
      3. 请求路径： path
      4. 请求参数： query
      5. 请求fragment: fragment
      6. 请求消息体： body
      7. 请求头部map： headers
      8. 请求参数map: params
      9. 请求cookiesMap: cookies
2. HttpReponse响应体
   1. 基本属性
      1. 响应状态： m_status
      2. http协议版本： m_version
      3. 是否自动关闭： m_close
      4. 响应消息体： body
      5. 响应原因： m_reason
      6. 响应头部： headers