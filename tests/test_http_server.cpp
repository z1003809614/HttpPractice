#include "../myhttp/http/http_server.h"
#include "../myhttp/log.h"


static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();


void run(){
    myhttp::http::HttpServer::ptr server(new myhttp::http::HttpServer);
    myhttp::Address::ptr addr = myhttp::Address::LookupAnyIPAddress("0.0.0.0:80");
    while(!server->bind(addr)){
        sleep(2);
    }
    server->start();
}

int main(){
    myhttp::IOManager iom(1);
    iom.schedule(run);
    return 0;
}