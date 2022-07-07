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
        // 这里的iovec只是用来获取bytearray中可用的地址信息；
        // 其实际的数据的内存开辟是在bytearray中完成的；
        std::vector<iovec> iovs;
        // 利用iov结构体，获得1024字节的数据；
        ba->getWriteBuffers(iovs, 1024);
        // 调用recv函数，读取数据到buffer中；
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
        // 这里这样使用是为了读取数据，但是写法很怪异，根本原因是bytearray的封装有问题；
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