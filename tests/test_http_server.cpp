#include "../myhttp/http/http_server.h"
#include "../myhttp/log.h"


static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();


void run(){
    myhttp::http::HttpServer::ptr server(new myhttp::http::HttpServer);
    myhttp::Address::ptr addr = myhttp::Address::LookupAnyIPAddress("0.0.0.0:80");
    while(!server->bind(addr)){
        sleep(2);
    }
    auto sd = server->getServletDispath();
    sd->addServlet("/myhttp/xx", [](myhttp::http::HttpRequest::ptr req
                                ,myhttp::http::HttpResponse::ptr rsp
                                ,myhttp::http::HttpSession::ptr session)->int32_t{
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/myhttp/*", [](myhttp::http::HttpRequest::ptr req
                                ,myhttp::http::HttpResponse::ptr rsp
                                ,myhttp::http::HttpSession::ptr session)->int32_t{
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });
    
    server->start();
}

int main(){
    myhttp::IOManager iom(1);
    iom.schedule(run);
    return 0;
}