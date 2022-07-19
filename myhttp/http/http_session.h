/**
 * @file http_session.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief  HTTPSession封装
 * @version 0.1
 * @date 2022-07-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_HTTP_SESSION_H__
#define __MYHTTP_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace myhttp
{
    namespace http
    {   
        /**
         * @brief HTTPSession封装
         */
        class HttpSession : public SocketStream{
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<HttpSession> ptr;
            
            /**
             * @brief 构造函数
             * @param[in] sock Socket类型
             * @param[in] owner 是否托管
             */
            HttpSession(Socket::ptr sock, bool owner = true);
            
            /**
             * @brief 接收HTTP请求
             *  接收socket的数据并进行分析处理，返回一个HttpRequest对象；
             */
            HttpRequest::ptr recvRequest();
            
            /**
             * @brief 发送HTTP响应
             * @param[in] rsp HTTP响应
             * @return >0 发送成功
             *         =0 对方关闭
             *         <0 Socket异常
             */
            int sendResponse(HttpResponse::ptr rsp);
        };
        
    } // namespace http
    
} // namespace myhttp



#endif