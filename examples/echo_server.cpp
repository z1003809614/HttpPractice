#include "../myhttp/tcp_server.h"
#include "../myhttp/iomanager.h"
#include "../myhttp/log.h"
#include "../myhttp/bytearray.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

class EchoServer : public myhttp::TcpServer{
    public:
        EchoServer(int type);
        void handleClient(myhttp::Socket::ptr client);
    
    private:
        int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type)
{
}

void EchoServer::handleClient(myhttp::Socket::ptr client){
    MYHTTP_LOG_INFO(g_logger) << "handleClient " << *client;
    myhttp::ByteArray::ptr ba(new myhttp::ByteArray);
    while(true){
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0){
            MYHTTP_LOG_INFO(g_logger) << " client close: " << *client;
            break;
        }
        else if(rt < 0){
            MYHTTP_LOG_INFO(g_logger) << " client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1){ // text
            MYHTTP_LOG_INFO(g_logger) << ba->toString();           
        }else{
            MYHTTP_LOG_INFO(g_logger) << ba->toHexString();
        }
    }
}

int type = 1;

void run(){
    EchoServer::ptr es(new EchoServer(type));
    auto addr = myhttp::Address::LookupAny("0.0.0.0:8020");
    while(!es->bind(addr)){
        sleep(2);
    }
    es->start();
}

int main(int argc, char** argv){
    if(argc < 2){
        MYHTTP_LOG_INFO(g_logger) << "user as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")){
        type = 2;
    }
    myhttp::IOManager iom(2);
    iom.schedule(run);
    return 0;
}