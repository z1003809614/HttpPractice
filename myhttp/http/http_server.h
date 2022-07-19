/**
 * @file http_server.h
 * @brief HTTP服务器封装
 * @author sylar.yin
 * @email 564628276@qq.com
 * @date 2019-06-09
 * @copyright Copyright (c) 2019年 sylar.yin All rights reserved (www.sylar.top)
 */

#ifndef __MYHTTP_HTTP_HTTP_SERVER_H__
#define __MYHTTP_HTTP_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace myhttp
{
    namespace http
    {   
        /**
         * @brief HTTP服务器类
         */
        class HttpServer : public TcpServer{
            public:
                typedef std::shared_ptr<HttpServer> ptr;

                /**
                 * @brief 构造函数
                 * @param[in] keepalive 是否长连接
                 * @param[in] worker 工作调度器
                 * @param[in] accept_worker 接收连接调度器
                 */
                HttpServer(bool keepalive = false,
                           myhttp::IOManager* worker = myhttp::IOManager::GetThis(),
                           myhttp::IOManager* accept_worker = myhttp::IOManager::GetThis());
            
                /**
                 * @brief 获取ServletDispatch
                 */
                ServletDispatch::ptr getServletDispath() const { return m_dispatch; }

                /**
                 * @brief 设置ServletDispatch
                 */
                void setServletDispath(ServletDispatch::ptr v) { m_dispatch = v; }
            
            protected:
                virtual void handleClient(Socket::ptr client) override;
            private:
                /// 是否支持长连接
                bool m_isKeepalive;
                
                /// Servlet分发器
                ServletDispatch::ptr m_dispatch;
        };
    } // namespace http
    
} // namespace myhttp


#endif