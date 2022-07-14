#include"../myhttp/myhttp.h"
#include"../myhttp/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

int sock = 0;

void test_fiber(){
    MYHTTP_LOG_INFO(g_logger) << "test_fiber sock=";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "124.223.24.104", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))){

    }else if(errno == EINPROGRESS){
        MYHTTP_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        myhttp::IOManager::GetThis()->addEvent(sock, myhttp::IOManager::READ, [](){
            MYHTTP_LOG_INFO(g_logger) << "read callback";
        });
        myhttp::IOManager::GetThis()->addEvent(sock, myhttp::IOManager::WRITE, [](){
            MYHTTP_LOG_INFO(g_logger) << "write callback";
            myhttp::IOManager::GetThis()->cancelEvent(sock, myhttp::IOManager::READ);
            close(sock);
        });
    }
    else{
        MYHTTP_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}


void test1(){  
    myhttp::IOManager iom(2);
    iom.schedule(&test_fiber);
}


myhttp::Timer::ptr s_timer;
void test_timer(){
    myhttp::IOManager iom(1);
   // 这里不能使用这种形式，因为s_timer是局部变量，内部传引用后，由于不是同一个线程执行，当前s_timer很可能在其他线程使用的时候，就被析构了，导致其内部数据是不能保证的，造成了野指针的问题；
   // myhttp::Timer::ptr s_timer = iom.addTimer(1000,[&s_timer](){}
    s_timer = iom.addTimer(1000,[](){
        MYHTTP_LOG_INFO(g_logger) << "hello timer";
        static int times = 1;
        if(times++ == 3){
            
            s_timer->reset(2000, true);
            //timer->cancel();
        }
    },true);
}

void test(){
    MYHTTP_LOG_INFO(g_logger) << " error";
}

int main(int argc, char** argv){
    //test1();
    //test_timer();
    
    myhttp::IOManager iom(2);
    iom.addEvent(0, myhttp::IOManager::Event::READ, &test);
    return 0;
}
