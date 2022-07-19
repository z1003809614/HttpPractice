/**
 * @file http_connection.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief HTTP客户端类
 * @version 0.1
 * @date 2022-07-11
 * 
 * 1. 与HttpSession类相对应，
 * 2. 发送request包，并提供方法能够解析uri，生成request对象，序列化进行发送；
 * 3. 接受response包，并简单解析其内容，能够解析chunck的body类型；
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_HTTP_CONNECTION_H__
#define __MYHTTP_HTTP_CONNECTION_H__

#include "../socket_stream.h"
#include "http.h"
#include "../uri.h"
#include "../thread.h"

#include <list>
#include <atomic>


namespace myhttp
{
    namespace http
    {
        /**
         * @brief Http响应结果封装类
         */
        struct HttpResult{
            /// 智能指针重定义
            typedef std::shared_ptr<HttpResult> ptr;

            /**
             * @brief 错误码定义
             */
            enum class Error{
                /// 正常
                OK = 0,
                /// 非法URL
                INVALID_URL = 1,
                /// 无法解析的HOST
                INVALID_HOST = 2,
                /// 连接失败
                CONNECT_FAIL = 3,
                /// 连接被对方关闭
                SEND_CLOSE_BY_PEER = 4,
                /// 发送请求产生Socket错误
                SEND_SOCKET_ERROR = 5,
                /// 超时
                TIMEOUT = 6,
                /// 创建Socket失败
                CREATE_SOCKET_ERROR = 7,
                /// 从连接池中取连接失败
                POOL_GET_CONNECTION = 8,
                /// 无效的连接
                POOL_INVALID_CONNECTION = 9,
            };

             /**
             * @brief 构造函数
             * @param[in] _result 错误码
             * @param[in] _response HTTP响应结构体
             * @param[in] _error 错误描述
             */
            HttpResult(int _result, HttpResponse::ptr _response, const std::string& _error)
                :result(_result)
                ,response(_response)
                ,error(_error){}
            
        
            /// 错误码
            int result;
            /// HTTP响应结构体
            HttpResponse::ptr response;
            /// 错误描述
            std::string error;

            /**
             * @brief 
             * @return std::string 
             */
            std::string toString() const;
        };

        class HttpConnectionPool;   // 提前进行声明否则下面的类不能使用；
        /**
         * @brief HTTP客户端类
         * 1. 用于发送request和接受response
         */
        class HttpConnection : public SocketStream{
        friend class HttpConnectionPool;
            public:

                /// HTTP客户端类智能指针
                typedef std::shared_ptr<HttpConnection> ptr;
                
                /**
                 * @brief 发送HTTP的GET请求
                 * @param[in] url 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoGet(const std::string& url
                                            ,uint64_t timeout_ms
                                            ,const std::map<std::string, std::string>& headers = {}
                                            ,const std::string& body = "");


                /**
                 * @brief 发送HTTP的GET请求
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoGet(Uri::ptr uri
                                            ,uint64_t timeout_ms
                                            ,const std::map<std::string, std::string>& headers = {}
                                            ,const std::string& body = "");

                /**
                 * @brief 发送HTTP的POST请求
                 * @param[in] url 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoPost(const std::string& url
                                            ,uint64_t timeout_ms
                                            ,const std::map<std::string, std::string>& headers = {}
                                            ,const std::string& body = "");

                /**
                 * @brief 发送HTTP的POST请求
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoPost(Uri::ptr uri
                                            ,uint64_t timeout_ms
                                            ,const std::map<std::string, std::string>& headers = {}
                                            ,const std::string& body = "");

                /**
                 * @brief 发送HTTP请求
                 * @param[in] method 请求类型
                 * @param[in] uri 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoRequest(HttpMethod method
                                                ,const std::string& url
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers = {}
                                                ,const std::string& body = "");


                /**
                 * @brief 发送HTTP请求
                 * @param[in] method 请求类型
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoRequest(HttpMethod method
                                                ,Uri::ptr uri
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers = {}
                                                ,const std::string& body = "");

                /**
                 * @brief 发送HTTP请求
                 * @param[in] req 请求结构体
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @return 返回HTTP结果结构体
                 */
                static HttpResult::ptr DoRequest(HttpRequest::ptr req
                                                ,Uri::ptr uri
                                                ,uint64_t timeout_ms);

                /**
                 * @brief 构造函数
                 * @param[in] sock Socket类
                 * @param[in] owner 是否掌握所有权
                 */
                HttpConnection(Socket::ptr sock, bool owner = true);
                
                /**
                 * @brief 析构函数
                 */
                ~HttpConnection();
                
                /**
                 * @brief  接收socket的数据并进行分析处理，返回一个HttpRequest对象；
                 * 
                 * @return HttpResponse::ptr,解析失败返回nullptr;
                 */
                HttpResponse::ptr recvResponse();
                
                /**
                 * @brief 把封装好的response对象序列化发送出去；
                 *   1.内部使用了ScoketStream类的方法
                 * @param[in] req HTTP请求结构 
                 * @return int 成功，返回发送的字节数；失败返回为非正数，核心函数为send;
                 */
                int sendRequest(HttpRequest::ptr req);

            private:
                /// 创建时间；
                uint64_t m_createTime = 0;
                /// 请求数量；感觉应该是被使用的次数，用于长连接的某些限制问题；
                uint64_t m_request = 0;
        };

        /**
         * @brief HTTP客户端连接池
         * 1. 一个连接池中，存有一个host和vhost,其内部的所有connection都只能请求host和vhost中的资源；
         * 2. 连接池中的url，其含义为path，并不是connection中的完整url；
         */
        class HttpConnectionPool{
            public:
                /// 智能指针重定义；
                typedef std::shared_ptr<HttpConnectionPool> ptr;
                /// 互斥量
                typedef Mutex MutexType;

                /**
                 * @brief 连接池构造函数
                 * @param host 域名或ip地址
                 * @param vhost 备用host
                 * @param port 端口号
                 * @param max_size 连接池的最大容量
                 * @param alive_time 一个连接的可存活时间
                 * @param max_request 一个连接的可使用最大次数
                 */
                HttpConnectionPool (const std::string& host
                                    ,const std::string& vhost
                                    ,uint32_t port
                                    ,uint32_t max_size
                                    ,uint32_t alive_time
                                    ,uint32_t max_request);
                
                /**
                 * @brief 从pool中获取一个connection
                 *  内部实现了对invalid_connection的清除；
                 * @return HttpConnection::ptr，失败返回nullptr;
                 */
                HttpConnection::ptr getConnection();
            

                /**
                 * @brief 发送HTTP的GET请求
                 * @param[in] url 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doGet(const std::string& url
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers = {}
                                    ,const std::string& body = "");

                /**
                 * @brief 发送HTTP的GET请求
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doGet(Uri::ptr uri
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers = {}
                                    ,const std::string& body = "");

                /**
                 * @brief 发送HTTP的POST请求
                 * @param[in] url 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doPost(const std::string& url
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers = {}
                                    ,const std::string& body = "");

                /**
                 * @brief 发送HTTP的POST请求
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doPost(Uri::ptr uri
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers = {}
                                    ,const std::string& body = "");

                /**
                 * @brief 发送HTTP请求
                 * @param[in] method 请求类型
                 * @param[in] uri 请求的url
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doRequest(HttpMethod method
                                        ,const std::string& url
                                        ,uint64_t timeout_ms
                                        ,const std::map<std::string, std::string>& headers = {}
                                        ,const std::string& body = "");

                /**
                 * @brief 发送HTTP请求
                 * @param[in] method 请求类型
                 * @param[in] uri URI结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @param[in] headers HTTP请求头部参数
                 * @param[in] body 请求消息体
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doRequest(HttpMethod method
                                        ,Uri::ptr uri
                                        ,uint64_t timeout_ms
                                        ,const std::map<std::string, std::string>& headers = {}
                                        ,const std::string& body = "");

                /**
                 * @brief 发送HTTP请求
                 *      该方法时发送request的核心方法
                 * @param[in] req 请求结构体
                 * @param[in] timeout_ms 超时时间(毫秒)
                 * @return 返回HTTP结果结构体
                 */
                HttpResult::ptr doRequest(HttpRequest::ptr req
                                        ,uint64_t timeout_ms);

            private:
                /**
                 * @brief 自定义connection智能指针的析构函数；
                 *      该函数主要是配合httpconnection的长连接属性(keep-alive)，
                 *      当connection未达到限制条件的时候，不会将其析构；
                 * @param[in] ptr 析构connection指针 
                 * @param[in] pool 该connection所在pool
                 */
                static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);

            private:
                
                /// 主host，即当前connection直接通信的host；
                std::string m_host;
                
                /// 备用host，其实含义与m_host一致；
                std::string m_vhost;
                
                /// 端口号
                uint32_t m_port;
                
                /// 连接池的最大容量
                uint32_t m_maxSize;
                
                /// 一个连接的最长存活时间
                uint32_t m_maxAliveTime;
                
                /// 一个连接的最多使用次数
                uint32_t m_maxRequest;

                /// 互斥量，用于pool添加和删除connection的时候，避免多线程造成数据错误；
                MutexType m_mutex;
                
                /// 链表容器，存储connection
                std::list<HttpConnection*> m_conns;
                
                /// 已经保存的connection数量，原子量，保证其数据一致性
                std::atomic<int32_t> m_total = {0};
        };
        
    } // namespace http
    
    
} // namespace myhttp



#endif