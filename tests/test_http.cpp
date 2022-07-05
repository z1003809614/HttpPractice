#include "../myhttp/http/http.h"
#include "../myhttp/log.h"

void test_request(){
    myhttp::http::HttpRequest::ptr req(new myhttp::http::HttpRequest);
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello sylar");
    req->dump(std::cout) << std::endl;
}

void test_response(){
    myhttp::http::HttpResponse::ptr rsp(new myhttp::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setStatus((myhttp::http::HttpStatus)400);
    rsp->setClose(false);
    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv){
    test_request();
    test_response();
    return 0;
}