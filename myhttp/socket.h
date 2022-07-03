#ifndef __MYHTTP_SOCKET_H__
#define __MYHTTP_SOCKET_H__

#include <memory>
#include "address.h"
#include "noncopyable.h"

namespace myhttp
{
    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
        public:
            typedef std::shared_ptr<Socket> ptr;
            typedef std::weak_ptr<Socket> weak_ptr;

            // 简化传输层宏定义
            enum Type{
                TCP = SOCK_STREAM,
                UDP = SOCK_DGRAM,
            };

            // 简化网络IP层的宏定义
            enum Family{
                IPv4 = AF_INET,
                IPv6 = AF_INET6,
                UNIX = AF_UNIX,
            };

            // 根据地址信息生成socket;
            static Socket::ptr CreateTCP(myhttp::Address::ptr address);
            static Socket::ptr CreateUDP(myhttp::Address::ptr address);

            static Socket::ptr CreateTCPSocket();
            static Socket::ptr CreateUDPSocket();

            static Socket::ptr CreateTCPSocket6();
            static Socket::ptr CreateUDPSocket6();

            static Socket::ptr CreateUnixTCPSocket();
            static Socket::ptr CreateUnixUDPSocket();

            Socket(int family, int type, int protocol = 0);
            ~Socket();

            // 使用了FdCtx类，来获取其超时时间的定义；
            int64_t getSendTimeout();
            void setSendTimeout(int64_t v);

            int64_t getRecvTimeout();
            void setRecvTimeout(int64_t v);

            // 获取当前socket,在level层级(网络层or传输层)上，option属性的值，存储到长度为len的result中；
            // 内部使用getsockopt()函数实现；
            bool getOption(int level, int option, void* result, size_t* len);
            template<class T>
            bool getOption(int level, int option, T& result){
                size_t length = sizeof(T);
                return getOption(level, option, &result, &length);
            }

            // 与getOption想对应，内部使用setsockopt来实现();
            bool setOption(int level, int option, const void* result, size_t len);
            template<class T>
            bool setOption(int level, int option, const T& value){
                return setOption(level, option, &value, sizeof(T));
            }

            // 接受请求当前socket的socket;
            Socket::ptr accept();

            // 将sock系统文件描述符，初始化到自身属性中 内部调用initSock()等函数；
            bool init(int sock);
            bool bind(const Address::ptr addr);
            bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
            bool listen(int backlog = SOMAXCONN);
            bool close();

            int send(const void* buffer, size_t length, int flags = 0);
            // 实际封装的是sendmsg
            int send(const iovec* buffers, size_t length, int flags = 0);
            int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
            int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

            int recv(void* buffer, size_t length, int flags = 0);
            int recv(iovec* buffers, size_t length, int flags = 0);
            int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
            int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

            // 获取远端的地址信息，如果没有会进行初始化m_remoteaddress
            Address::ptr getRemoteAddress();
            // 获取本地的地址信息，如果没有会进行初始化m_localaddress
            Address::ptr getLocalAddress();

            int getFamily() const { return m_family; }
            int getType() const { return m_type; }
            int getProtocol() const{ return m_protocol; }

            bool isConnected() const { return m_isConnected; }
            // 该类初始情况没有对应一个实际的sockfd，默认其fd属性为-1；
            bool isValid() const;
            // 获取当前socket的error出错情况，内部使用getOption函数；
            int getError();

            // 字符串化socket对象
            std::ostream&  dump(std::ostream& os) const;
            int getSocket() const { return m_sock; }

            // 调用IOManager的对应的cancel系列函数；
            bool cancelRead();
            bool cancelWrite();
            bool cancelAccept();
            bool cancelAll();

        private:
            // 调用setOption函数，设置当前套接字的属性，默认使用REUSEADDR属性
            // 如果是TCP的话，开启TCP_NODELAY属性，提供广域网传输效率；
            void initSock();
            // 调用socket()系统函数，并使用initSock()来进行初始化；
            void newSock();
        private:
            int m_sock;
            int m_family;
            int m_type;
            int m_protocol;
            bool m_isConnected;

            Address::ptr m_localAddress;
            Address::ptr m_remoteAddress;
    };
} // namespace myhttp


#endif
