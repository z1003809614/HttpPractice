#include "http_connection.h"
#include "http_parser.h"
#include "../log.h"
#include "../socket.h"

namespace myhttp
{
    namespace http
    {

        static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system"); 

        std::string HttpResult::toString() const{
            std::stringstream ss;
            ss << "[HttpResult result=" << result
                << " error=" << error
                << " response=" << (response ? response->toString() : "nullptr")
                << "]";
            return ss.str();
        }

//===========================HttpConnection=============================================

        HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
            :SocketStream(sock, owner){
        }

        HttpConnection::~HttpConnection(){
                MYHTTP_LOG_INFO(g_logger) << " HttpConnection destruct";
        }

        HttpResponse::ptr HttpConnection::recvResponse(){
            HttpResponseParser::ptr parser(new HttpResponseParser);
            
            uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();
            
            std::shared_ptr<char> buffer(new char[buff_size + 1], [](char* ptr){
                delete[] ptr;
            });

            char* data = buffer.get();
            
            int offset = 0; 

            // 解析haeder部分
            do{
                int len = read(data + offset, buff_size - offset);
                if(len <= 0){
                    MYHTTP_LOG_ERROR(g_logger) << "read length <= 0";
                    return nullptr;
                }

                len += offset;
                
                data[len] = '\0';
                
                size_t nparser = parser->execute(data, len, false);
                if(parser->hasError()){
                    MYHTTP_LOG_ERROR(g_logger) << "parser has error";
                    return nullptr;
                }

                offset = len - nparser;

                if(offset == (int)buff_size){
                    MYHTTP_LOG_ERROR(g_logger) << "offset == buff_size";
                    return nullptr;
                }

                if(parser->isFinished()){
                    break;
                }
            }while(true);

            // 解析body部分,如果头部解析得到body使用chunck模式，则需要额外进行处理；
            // chunck编码：[content-length]\r\n[data]\r\n;
            auto& client_parser = parser->getParser();
            if(client_parser.chunked){
                std::string body;
                int len = offset;
                do{ // 每次执行读取一行chunck；
                    do{ // 每次循环结束能够解析一个chunck的头部信息，即content-length;
                        int rt = read(data + len, buff_size - len);
                        if(rt <= 0){
                            return nullptr;
                        }
                        len += rt;
                        data[len] = '\0';
                        size_t nparse = parser->execute(data, len, true);
                        if(parser->hasError()){
                            return nullptr;
                        }
                        len -= nparse;
                        if(len == (int)buff_size){
                            return nullptr;
                        }
                    }while(!parser->isFinished());
                    
                    len -= 2; // chunck-data会给存储的原始数据中额外加上一个\r\n，所有这里存储body的时候需要手动去掉；

                    if(client_parser.content_len <= len){ // body长度小于len时，表示body数据已经读取到data中；

                        body.append(data, client_parser.content_len);
                        
                        memmove(data, data + client_parser.content_len, len - client_parser.content_len);
                        
                        len -= client_parser.content_len;

                    }else{  // body数据还未读取到data中；
                        body.append(data, len);
                        int left = client_parser.content_len - len;
                        while(left > 0){
                            int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                            if(rt <= 0){
                                return nullptr;
                            }
                            body.append(data, rt);
                            left -= rt;
                        }
                        len = 0;
                    }
                }while(!client_parser.chunks_done);

                parser->getData()->setBody(body);

            }else{
                int64_t length = parser->getContentLength();
                if(length > 0){
                    std::string body;
                    body.resize(length);
                    
                    int len = 0;
                    if(length >= offset){
                        memcpy(&body[0], data, offset);
                        len = offset;
                    }else{
                        memcpy(&body[0], data, length);
                        len = length;
                    }
                    length -= offset;
                    if(length > 0){
                        if(readFixSize(&body[len], length) <= 0 ){
                            MYHTTP_LOG_ERROR(g_logger) << "readFixSize <= 0";
                            return nullptr;
                        } 
                    }
                    parser->getData()->setBody(body);
                }
            }
            return parser->getData();
        }

        int HttpConnection::sendRequest(HttpRequest::ptr req){
            std::stringstream ss;
            ss << *req;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }



        HttpResult::ptr HttpConnection::DoGet(const std::string& url
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers 
                                    ,const std::string& body )
        {
            Uri::ptr uri = Uri::Create(url);
            if(!uri){
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                        , nullptr, "invalid url: " + url);
            }
            return DoGet(uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers 
                                    ,const std::string& body)
        {
            return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoPost(const std::string& url
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers
                                    ,const std::string& body)
        {
            Uri::ptr uri = Uri::Create(url);
            if(!uri){
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                        , nullptr, "invalid url: " + url);
            }
            return DoPost(uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri
                                    ,uint64_t timeout_ms
                                    ,const std::map<std::string, std::string>& headers 
                                    ,const std::string& body)
        {
            return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                                        ,const std::string& url
                                        ,uint64_t timeout_ms
                                        ,const std::map<std::string, std::string>& headers
                                        ,const std::string& body)
        {
            Uri::ptr uri = Uri::Create(url);
            if(!uri){
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                        , nullptr, "invalid url: " + url);
            }
            return DoRequest(method, uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                                        ,Uri::ptr uri
                                        ,uint64_t timeout_ms
                                        ,const std::map<std::string, std::string>& headers 
                                        ,const std::string& body)
        {
            HttpRequest::ptr req = std::make_shared<HttpRequest>();
            req->setPath(uri->getPath());
            req->setQuery(uri->getQuery());
            req->setFragment(uri->getFragment());
            req->setMethod(method);
            bool has_host = false;
            for(auto& i : headers){
                if(strcasecmp(i.first.c_str(), "connection") == 0){
                    if(strcasecmp(i.second.c_str(), "keep-alive") == 0){
                        req->setClose(false);
                    }
                    continue;
                }

                if(!has_host && strcasecmp(i.first.c_str(), "host") == 0){
                    has_host = !i.second.empty();
                }
                req->setHeader(i.first, i.second);
            }
            
            // 如果没有设定host，就使用uri解析到的host；
            if(!has_host){
                req->setHeader("Host", uri->getHost());
            }
            req->setBody(body);
            return DoRequest(req, uri, timeout_ms);
        }

        HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req
                                        ,Uri::ptr uri
                                        ,uint64_t timeout_ms)
        {
            // uri，存储的host是要直接访问的host，而req中的host则是我实际需要请求的host；
            // 由于存在代理转发的情况，所以uri->host和req的host是可能不一致的；
            Address::ptr addr = uri->createAddress();
            if(!addr){
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST
                        ,nullptr, "invalid host: " + uri->getHost());
            }
            
            Socket::ptr sock = Socket::CreateTCP(addr);
            if(!sock){
                return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR
                        ,nullptr, "create socket fail: " + addr->toString()
                        + " errno=" + std::to_string(errno)
                        + " errstr=" + std::string(strerror(errno)));
            }
            
            if(!sock->connect(addr)){
                return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL
                        ,nullptr, "connect fail: " + addr->toString());
            }

            sock->setRecvTimeout(timeout_ms);
            HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
            
            int rt = conn->sendRequest(req);
            if(rt == 0){
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                        ,nullptr, "send request closed by peer: " + addr->toString());
            }
            if(rt < 0){
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                        ,nullptr, "send request socket error errno= " + addr->toString()
                        + " errstr=" + std::string(strerror(errno)));
            }

            auto rsp = conn->recvResponse();
            
            if(!rsp){
                 return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                        ,nullptr, "recv response timeout: " + addr->toString()
                        + " timeout_ms:" + std::to_string(timeout_ms));
            }

            return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
        }

//==========================================HttpConnectionPool===============================================

        HttpConnectionPool::HttpConnectionPool(const std::string& host
                                            ,const std::string& vhost
                                            ,uint32_t port
                                            ,uint32_t max_size
                                            ,uint32_t alive_time
                                            ,uint32_t max_request)
            :m_host(host)
            ,m_vhost(vhost)
            ,m_port(port)
            ,m_maxSize(max_size)
            ,m_maxAliveTime(alive_time)
            ,m_maxRequest(max_request)
        {}


        HttpConnection::ptr HttpConnectionPool::getConnection(){
            
            // 获取当前的时间
            uint64_t now_ms = myhttp::GetCurrentMS();
            
            // 存放连接池中失效的connection；
            std::vector<HttpConnection*> invalid_conns;
            
            // 用于返回connection
            HttpConnection* ptr = nullptr;

            MutexType::Lock lock(m_mutex);
            
            // 连接池中connection数量大于0的时候；
            while(!m_conns.empty()){
                // 获取第一个connection;
                auto conn = *m_conns.begin();
                m_conns.pop_front();

                // 如果connection连接已被关闭；
                if(!conn->isConnected()){
                    // 加入失效连接池中
                    invalid_conns.push_back(conn);
                    continue;
                }
                // connection的时间限制超时也将其回收；感觉这里逻辑应该是"<";
                if((conn->m_createTime + m_maxAliveTime) > now_ms){
                    invalid_conns.push_back(conn);
                    continue;
                }
                ptr = conn;
                break;
            }
            
            lock.unlock();
            
            // 释放所有失效连接
            for(auto i : invalid_conns){
                delete i;
            }
            // 修改连接总数；
            m_total -= invalid_conns.size();

            // 当没有合适的connection对象的时候，就创造一个新的connection对象；
            if(!ptr){
                // 根据pool的host属性，获取address；
                IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
                if(!addr){
                    MYHTTP_LOG_ERROR(g_logger) << "get addr fail: " << m_host;
                    return nullptr;
                }
                // 设置port属性；
                addr->setPort(m_port);
                // 根据addr生成socket；
                myhttp::Socket::ptr sock = myhttp::Socket::CreateTCP(addr);
                // sock生成失败
                if(!sock){
                    MYHTTP_LOG_ERROR(g_logger) << "create sock fail: " << addr->toString();
                    return nullptr;
                }
                // sock进行connect;
                if(!sock->connect(addr)){
                    MYHTTP_LOG_ERROR(g_logger) << "sock connect fail: " << addr->toString();
                    return nullptr;
                }

                // 使用该sock构建Connection；
                ptr = new HttpConnection(sock);
                ++m_total;
            }

            // ptr是裸指针，构建一个智能指针，并使用bind设置析构函数；
            return HttpConnection::ptr(ptr, std::bind(&HttpConnectionPool::ReleasePtr
                                        , std::placeholders::_1, this));

            // std::bind记录，第一个参数是绑定的函数，后续是绑定函数的参数，
            // std::placeholders::_1，表示占位，即需要调用者传递参数，_1表示占位第几个？
            // std::bind，默认以值传递参数，如果需要引用，则需要使用ref()函数；
        }

        HttpResult::ptr HttpConnectionPool::doGet(const std::string& url
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers
                                                ,const std::string& body)
        {
            return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers
                                                ,const std::string& body)
        {
            std::stringstream ss;

            ss << uri->getPath()
               << (uri->getQuery().empty() ? "" : "?")
               << uri->getQuery()
               << (uri->getFragment().empty() ? "" : "#")
               << uri->getFragment();
            
            return doGet(ss.str(), timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnectionPool::doPost(const std::string& url
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers
                                                ,const std::string& body)
        {
            return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri
                                                ,uint64_t timeout_ms
                                                ,const std::map<std::string, std::string>& headers
                                                ,const std::string& body)
        {
            std::stringstream ss;

            ss << uri->getPath()
               << (uri->getQuery().empty() ? "" : "?")
               << uri->getQuery()
               << (uri->getFragment().empty() ? "" : "#")
               << uri->getFragment();
            
            return doPost(ss.str(), timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
                                                    ,const std::string& url
                                                    ,uint64_t timeout_ms
                                                    ,const std::map<std::string, std::string>& headers
                                                    ,const std::string& body)
        {
            HttpRequest::ptr req = std::make_shared<HttpRequest>();
            req->setPath(url);
            req->setMethod(method);
            // 默认设置为短连接；
            req->setClose(true);
            // 用于标识header中是否存在host字段；
            bool has_host = false;
            for(auto& i : headers){
                // strcasecmp,若参数s1 和s2 字符串相同则返回0；s1 长度大于s2 长度则返回大于0 的值，s1 长度若小于s2 长度则返回小于0 的值
                // 进一步判断是否应为长连接；
                if(strcasecmp(i.first.c_str(), "connection") == 0){
                    if(strcasecmp(i.second.c_str(), "keep-alive") == 0){
                        // 设置为长连接
                        req->setClose(false);
                    }
                    continue;
                }

                // 存在host字段；
                if(!has_host && strcasecmp(i.first.c_str(), "host") == 0){
                    has_host = !i.second.empty();
                }
                // 设置header;
                req->setHeader(i.first, i.second);
            }
            
            // 当不存在host字段时，使用pool中的m_host或m_vhost;
            if(!has_host){
                if(m_vhost.empty()){
                    req->setHeader("Host", m_host);    
                }
                else{
                    req->setHeader("Host", m_vhost);
                }
            }
            req->setBody(body);
            return doRequest(req, timeout_ms);
        }

        HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
                                                    ,Uri::ptr uri
                                                    ,uint64_t timeout_ms
                                                    ,const std::map<std::string, std::string>& headers 
                                                    ,const std::string& body)
        {
            std::stringstream ss;

            ss << uri->getPath()
               << (uri->getQuery().empty() ? "" : "?")
               << uri->getQuery()
               << (uri->getFragment().empty() ? "" : "#")
               << uri->getFragment();
            
            return doRequest(method, ss.str(), timeout_ms, headers, body);
        }


        HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req
                                                    ,uint64_t timeout_ms)
        {
            auto conn = getConnection();
            // 无法获取connection，返回错误；
            if(!conn){
                return std::make_shared<HttpResult> ((int)HttpResult::Error::POOL_GET_CONNECTION
                            , nullptr, "pool host: " + m_host + " port:" + std::to_string(m_port));
            }
            
            // 获取conn的sockfd；
            auto sock = conn->getSocket();
            if(!sock){
                return std::make_shared<HttpResult> ((int)HttpResult::Error::POOL_INVALID_CONNECTION
                            , nullptr, "pool host: " + m_host + " port:" + std::to_string(m_port));    
            }

            // sock设置接受超时时间；
            sock->setRecvTimeout(timeout_ms);
            // 发送request;
            int rt = conn->sendRequest(req);

            if(rt == 0){
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                        ,nullptr, "send request closed by peer: " + sock->getRemoteAddress()->toString());
            }
            if(rt < 0){
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                        ,nullptr, "send request socket error errno= " + sock->getRemoteAddress()->toString()
                        + " errstr=" + std::string(strerror(errno)));
            }

            // 接受response;
            auto rsp = conn->recvResponse();
            
            if(!rsp){
                 return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                        ,nullptr, "recv response timeout: " + sock->getRemoteAddress()->toString()
                        + " timeout_ms:" + std::to_string(timeout_ms));
            }

            return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
        }

        void HttpConnectionPool::ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool){
            ++(ptr->m_request);
            if(!ptr->isConnected()
                || ((ptr->m_createTime + pool->m_maxAliveTime) >= myhttp::GetCurrentMS())
                || (ptr->m_request > pool->m_maxRequest)){
                delete ptr;
                --(pool->m_total);
                return;
            }
            MutexType::Lock lock(pool->m_mutex);
            pool->m_conns.push_back(ptr);
        }
    } // namespace http
    
} // namespace myhttp
