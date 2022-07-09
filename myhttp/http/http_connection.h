#ifndef __MYHTTP_HTTP_CONNECTION_H__
#define __MYHTTP_HTTP_CONNECTION_H__

#include "../socket_stream.h"
#include "http.h"

namespace myhttp
{
    namespace http
    {
        class HttpConnection : public SocketStream{
        public:
            typedef std::shared_ptr<HttpConnection> ptr;
            
            HttpConnection(Socket::ptr sock, bool owner = true);
            
            // 接收socket的数据并进行分析处理，返回一个HttpRequest对象；
            HttpResponse::ptr recvResponse();
            
            // 把封装好的response对象序列化发送出去；
            int sendRequest(HttpRequest::ptr req);
        };
        
    } // namespace http
    
    
} // namespace myhttp



#endif