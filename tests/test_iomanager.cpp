#include"../myhttp/myhttp.h"
#include"../myhttp/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_fiber(){
    MYHTTP_LOG_INFO(g_logger) << "test fiber";
}

void test1(){
    myhttp::IOManager iom(2);
    iom.schedule(&test_fiber);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "124.223.24.104", &addr.sin_addr.s_addr);

    iom.addEvent(sock, myhttp::IOManager::READ, [](){
        MYHTTP_LOG_INFO(g_logger) << "connected";
    });
    iom.addEvent(sock, myhttp::IOManager::WRITE, [](){
        MYHTTP_LOG_INFO(g_logger) << "connected";
    });
    connect(sock, (const sockaddr*)&addr, sizeof(addr));
}

int main(int argc, char** argv){
    test1();
    return 0;
}
