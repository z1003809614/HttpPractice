# Servlet模块

## 模块设计
1. Servlet抽象基类
   1. 成员变量 m_name
   2. 提供接口handle()，用于处理实际的逻辑业务
2. FunctionServlet： 使用回调函数来重写handle业务
3. ServletDispathc: servlet分发器，相当于管理类
   1. 保存 <uri, servlet> 集合；
   2. 提供匹配方法，优先精准匹配，然后再使用fnmatch()来模糊匹配