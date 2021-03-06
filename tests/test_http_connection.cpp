#include <iostream>
#include "../myhttp/http/http_connection.h"
#include "../myhttp/log.h"
#include "../myhttp/iomanager.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_pool(){
    myhttp::http::HttpConnectionPool::ptr pool(new myhttp::http::HttpConnectionPool(
            "www.sylar.top", "", 80, 10, 1000 * 30, 5));

    myhttp::IOManager::GetThis()->addTimer(1000, [pool](){
        auto r = pool->doGet("/", 300);
        MYHTTP_LOG_INFO(g_logger) << r->toString();
    }, true);
}


void run(){
    myhttp::Address::ptr addr = myhttp::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr){
        MYHTTP_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    myhttp::Socket::ptr sock = myhttp::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt){
        MYHTTP_LOG_INFO(g_logger) << "connect " << addr->toString() << " failed";
        return;
    }

    myhttp::http::HttpConnection::ptr conn(new myhttp::http::HttpConnection(sock));
    myhttp::http::HttpRequest::ptr req(new myhttp::http::HttpRequest());
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    
    MYHTTP_LOG_INFO(g_logger) << " req: " << std::endl << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp){
        MYHTTP_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    // MYHTTP_LOG_INFO(g_logger) << "rsp: " << std::endl << *rsp;

    MYHTTP_LOG_INFO(g_logger) << "=====================================";

    auto r = myhttp::http::HttpConnection::DoGet("http://www.sylar.top/", 300);
   
    MYHTTP_LOG_INFO(g_logger) << "result=" << r->result
                              << " error=" << r->error
                              << " rsp=" << (rsp ? rsp->toString() : "");

    MYHTTP_LOG_INFO(g_logger) << "=====================================";

    test_pool();
}

int main(int argc, char** argv){
    myhttp::IOManager iom(1);
    iom.schedule(run);
    
    return 0;
}