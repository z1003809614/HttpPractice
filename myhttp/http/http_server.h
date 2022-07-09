#ifndef __MYHTTP_HTTP_HTTP_SERVER_H__
#define __MYHTTP_HTTP_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace myhttp
{
    namespace http
    {
        class HttpServer : public TcpServer{
            public:
                typedef std::shared_ptr<HttpServer> ptr;

                HttpServer(bool keepalive = false,
                           myhttp::IOManager* worker = myhttp::IOManager::GetThis(),
                           myhttp::IOManager* accept_worker = myhttp::IOManager::GetThis());
            
                ServletDispatch::ptr getServletDispath() const { return m_dispatch; }

                void setServletDispath(ServletDispatch::ptr v) { m_dispatch = v; }
            
            protected:
                virtual void handleClient(Socket::ptr client) override;
            
            
            private:
                bool m_isKeepalive;

                ServletDispatch::ptr m_dispatch;
                           
        };
    } // namespace http
    
} // namespace myhttp


#endif