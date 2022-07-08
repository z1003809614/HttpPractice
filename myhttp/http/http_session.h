#ifndef __MYHTTP_HTTP_SESSION_H__
#define __MYHTTP_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace myhttp
{
    namespace http
    {
        class HttpSession : public SocketStream{
        public:
            typedef std::shared_ptr<HttpSession> ptr;
            
            HttpSession(Socket::ptr sock, bool owner = true);
            
            // 接收socket的数据并进行分析处理，返回一个HttpRequest对象；
            HttpRequest::ptr recvRequest();
            
            // 把封装好的response对象序列化发送出去；
            int sendResponse(HttpResponse::ptr rsp);
        };
        
    } // namespace http
    
    
} // namespace myhttp



#endif