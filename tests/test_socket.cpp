#include "../myhttp/socket.h"
#include "../myhttp/myhttp.h"
#include "../myhttp/iomanager.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_socket(){
    myhttp::IPAddress::ptr addr = myhttp::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr){
        MYHTTP_LOG_INFO(g_logger) << "get address: " << addr->toString();
    }else{
        MYHTTP_LOG_INFO(g_logger) << "get address fail";
        return;
    }

    myhttp::Socket::ptr sock = myhttp::Socket::CreateTCP(addr);
    addr->setPort(80);
    if(!sock->connect(addr)){
        MYHTTP_LOG_ERROR(g_logger) << "connect " << addr->toString() << "fail";
    }else{
        MYHTTP_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0){
        MYHTTP_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0){
        MYHTTP_LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    MYHTTP_LOG_INFO(g_logger) << buffs;
}

int main(int argc, char** argv){
    myhttp::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}