#include "http_server.h"
#include "../log.h"

namespace myhttp
{
    namespace http
    {

        static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive,
                                myhttp::IOManager* worker,
                                myhttp::IOManager* accept_worker)
            :TcpServer(worker, accept_worker),
            m_isKeepalive(keepalive)
        {
            m_dispatch.reset(new ServletDispatch);
        }

        void HttpServer::handleClient(Socket::ptr client){
            
            HttpSession::ptr session(new HttpSession(client));
            do{
                auto req = session->recvRequest();
                if(!req){
                    MYHTTP_LOG_WARN(g_logger) << "recv http request fail, errno="
                        << errno << " errstr=" << strerror(errno)
                        << " client:"  << *client;
                    break;
                }
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
                
                // 使用servlet来处理request并设置好response;
                m_dispatch->handle(req, rsp, session);

                // rsp->setBody("hello myhttp");

                // MYHTTP_LOG_INFO(g_logger) << "request: " << std::endl
                //     << *req;

                // MYHTTP_LOG_INFO(g_logger) << "response: " << std::endl
                //     << *rsp;

                session->sendResponse(rsp);
            
            }while(m_isKeepalive);

            session->close();
        }
    } // namespace http
    
} // namespace myhttp
