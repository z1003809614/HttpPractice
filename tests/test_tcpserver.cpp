#include "../myhttp/tcp_server.h"
#include "../myhttp/iomanager.h"
#include "../myhttp/log.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void run(){
    auto addr = myhttp::Address::LookupAny("0.0.0.0:8033");
    auto addr2 = myhttp::UnixAddress::ptr(new myhttp::UnixAddress("/tmp/unix_addr"));
    std::vector<myhttp::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);
    
    myhttp::TcpServer::ptr tcp_server(new myhttp::TcpServer);

    std::vector<myhttp::Address::ptr> fails;
    while(!tcp_server->bind(addrs,fails)){
        sleep(2);
    }
    tcp_server->start();
}

int main(int argc, char** argv){
    myhttp::IOManager iom(2);
    iom.schedule(run);
    return 0;
}