#ifndef __MYHTTP_TCP_SERVER_H__
#define __MYHTTP_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "iomanager.h"
#include "address.h"
#include "socket.h"
#include "noncopyable.h"

namespace myhttp
{
    /**
     * @brief 封装了TcpServer的启动与运行相关函数
     * server类的核心是封装 socket的地址绑定bind，和listen函数，接受发起请求的sock；
     */
    class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable{
        public:
            typedef std::shared_ptr<TcpServer> ptr;
            TcpServer(myhttp::IOManager* woker = myhttp::IOManager::GetThis(),
                    myhttp::IOManager* accept_worker = myhttp::IOManager::GetThis());
            virtual ~TcpServer(){}
            
            // 地址绑定(bind + listen)(内部直接生成一一对应的socket)
            virtual bool bind(myhttp::Address::ptr addr);
            virtual bool bind(const std::vector<Address::ptr>& addrs, 
                              std::vector<Address::ptr>& fails);
            virtual bool start();
            virtual void stop();

            uint64_t getRecvTimeout() const { return m_recvTimeout; }
            std::string getName() const { return m_name; }
            void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
            void setName(const std::string& v) { m_name = v; }

            bool isStop() const { return m_isStop; }
        
        protected:
            // 对client进行处理
            virtual void handleClient(Socket::ptr client);
            // 对listen的sock的请求队列调用accept
            virtual void startAccept(Socket::ptr sock);
        private:
            std::vector<Socket::ptr> m_socks;
            IOManager* m_worker;                // 用于处理handle
            IOManager* m_acceptworker;          // 用于处理accept
            uint64_t m_recvTimeout;
            std::string m_name;                 // 区分不同的server实例
            bool m_isStop;
    };
} // namespace myhttp



#endif