/**
 * @file socket.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief Socket封装
 * @version 0.1
 * @date 2022-07-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_SOCKET_H__
#define __MYHTTP_SOCKET_H__

#include <memory>
#include "address.h"
#include "noncopyable.h"

namespace myhttp
{
    /**
     * @brief Socket封装类
     */
    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
        public:
            typedef std::shared_ptr<Socket> ptr;
            typedef std::weak_ptr<Socket> weak_ptr;

            /**
             * @brief Socket类型
             */
            enum Type{
                /// TCP类型
                TCP = SOCK_STREAM,
                
                /// UDP类型
                UDP = SOCK_DGRAM
            };

            /**
             * @brief Socket协议簇
             */
            enum Family {
                /// IPv4 socket
                IPv4 = AF_INET,
                
                /// IPv6 socket
                IPv6 = AF_INET6,
                
                /// Unix socket
                UNIX = AF_UNIX,
            };

            /**
             * @brief 创建TCP Socket(满足地址类型)
             * @param[in] address 地址
             */
            static Socket::ptr CreateTCP(myhttp::Address::ptr address);
            
            /**
             * @brief 创建UDP Socket(满足地址类型)
             * @param[in] address 地址
             */
            static Socket::ptr CreateUDP(myhttp::Address::ptr address);

            /**
             * @brief 创建IPv4的TCP Socket
             */
            static Socket::ptr CreateTCPSocket();
            
            /**
             * @brief 创建IPv4的UDP Socket
             */
            static Socket::ptr CreateUDPSocket();

            /**
             * @brief 创建IPv6的TCP Socket
             */
            static Socket::ptr CreateTCPSocket6();
            
            /**
             * @brief 创建IPv6的UDP Socket
             */
            static Socket::ptr CreateUDPSocket6();

            /**
             * @brief 创建Unix的TCP Socket
             */
            static Socket::ptr CreateUnixTCPSocket();
            
            /**
             * @brief 创建Unix的UDP Socket
             */
            static Socket::ptr CreateUnixUDPSocket();

            /**
             * @brief Socket构造函数
             * @param[in] family 协议簇
             * @param[in] type 类型
             * @param[in] protocol 协议
             */
            Socket(int family, int type, int protocol = 0);
            
            /**
             * @brief 析构函数
             */
            ~Socket();
            

            /**
             * @brief 获取发送超时时间(毫秒)
             * 使用了FdCtx类，来获取其超时时间的定义；
             */
            int64_t getSendTimeout();
            
            /**
             * @brief 设置发送超时时间(毫秒)
             */
            void setSendTimeout(int64_t v);

            /**
             * @brief 获取接受超时时间(毫秒)
             */
            int64_t getRecvTimeout();
            
            /**
             * @brief 设置接受超时时间(毫秒)
             */
            void setRecvTimeout(int64_t v);

            /**
             * @brief 获取sockopt @see getsockopt
             * 获取当前socket,在level层级(网络层or传输层)上，option属性的值，存储到长度为len的result中；
             * 内部使用getsockopt()函数实现；
             */
            bool getOption(int level, int option, void* result, size_t* len);

            /**
             * @brief 获取sockopt模板 @see getsockopt
             */
            template<class T>
            bool getOption(int level, int option, T& result){
                size_t length = sizeof(T);
                return getOption(level, option, &result, &length);
            }

            /**
             * @brief 设置sockopt @see setsockopt
             * 与getOption想对应，内部使用setsockopt来实现();
             */
            bool setOption(int level, int option, const void* result, size_t len);
            
            /**
             * @brief 设置sockopt模板 @see setsockopt
             */
            template<class T>
            bool setOption(int level, int option, const T& value){
                return setOption(level, option, &value, sizeof(T));
            }

            /**
             * @brief 接收connect链接
             * 接受请求当前socket的socket;
             * @return 成功返回新连接的socket,失败返回nullptr
             * @pre Socket必须 bind , listen  成功
             */
            Socket::ptr accept();

            
            bool init(int sock);
            
            /**
             * @brief 绑定地址
             * 将sock系统文件描述符，初始化到自身属性中 内部调用initSock()等函数；
             * @param[in] addr 地址
             * @return 是否绑定成功
             */
            bool bind(const Address::ptr addr);
            
            /**
             * @brief 连接地址
             * @param[in] addr 目标地址
             * @param[in] timeout_ms 超时时间(毫秒)
             */
            bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
            
            /**
             * @brief 监听socket
             * @param[in] backlog 未完成连接队列的最大长度
             * @result 返回监听是否成功
             * @pre 必须先 bind 成功
             */
            bool listen(int backlog = SOMAXCONN);
            
            /**
             * @brief 关闭socket
             */
            bool close();

            /**
             * @brief 发送数据
             * @param[in] buffer 待发送数据的内存
             * @param[in] length 待发送数据的长度
             * @param[in] flags 标志字
             * @return
             *      @retval >0 发送成功对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int send(const void* buffer, size_t length, int flags = 0);
            
            /**
             * @brief 发送数据
             * @param[in] buffers 待发送数据的内存(iovec数组)
             * @param[in] length 待发送数据的长度(iovec长度)
             * @param[in] flags 标志字
             * @return
             *      @retval >0 发送成功对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int send(const iovec* buffers, size_t length, int flags = 0);
            
            /**
             * @brief 发送数据
             * @param[in] buffer 待发送数据的内存
             * @param[in] length 待发送数据的长度
             * @param[in] to 发送的目标地址
             * @param[in] flags 标志字
             * @return
             *      @retval >0 发送成功对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
            
            /**
             * @brief 发送数据
             * @param[in] buffers 待发送数据的内存(iovec数组)
             * @param[in] length 待发送数据的长度(iovec长度)
             * @param[in] to 发送的目标地址
             * @param[in] flags 标志字
             * @return
             *      @retval >0 发送成功对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);
            
            /**
             * @brief 接受数据
             * @param[out] buffer 接收数据的内存
             * @param[in] length 接收数据的内存大小
             * @param[in] flags 标志字
             * @return
             *      @retval >0 接收到对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int recv(void* buffer, size_t length, int flags = 0);
            
            /**
             * @brief 接受数据
             * @param[out] buffers 接收数据的内存(iovec数组)
             * @param[in] length 接收数据的内存大小(iovec数组长度)
             * @param[in] flags 标志字
             * @return
             *      @retval >0 接收到对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int recv(iovec* buffers, size_t length, int flags = 0);
            
            /**
             * @brief 接受数据
             * @param[out] buffer 接收数据的内存
             * @param[in] length 接收数据的内存大小
             * @param[out] from 发送端地址
             * @param[in] flags 标志字
             * @return
             *      @retval >0 接收到对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
            
            /**
             * @brief 接受数据
             * @param[out] buffers 接收数据的内存(iovec数组)
             * @param[in] length 接收数据的内存大小(iovec数组长度)
             * @param[out] from 发送端地址
             * @param[in] flags 标志字
             * @return
             *      @retval >0 接收到对应大小的数据
             *      @retval =0 socket被关闭
             *      @retval <0 socket出错
             */
            int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

            /**
             * @brief 获取远端地址
             * 获取远端的地址信息，如果没有会进行初始化m_remoteaddress
             */
            Address::ptr getRemoteAddress();
            
            /**
             * @brief 获取本地地址
             * 获取本地的地址信息，如果没有会进行初始化m_localaddress
             */
            Address::ptr getLocalAddress();

            /**
             * @brief 获取协议簇
             */
            int getFamily() const { return m_family; }
            
            /**
             * @brief 获取类型
             */
            int getType() const { return m_type; }
            
            /**
             * @brief 获取协议
             */
            int getProtocol() const{ return m_protocol; }

            /**
             * @brief 返回是否连接
             */
            bool isConnected() const { return m_isConnected; }
            
            /**
             * @brief 是否有效(m_sock != -1)
             * 该类初始情况没有对应一个实际的sockfd，默认其fd属性为-1；
             */
            bool isValid() const;
            
            /**
             * @brief 返回Socket错误
             * 获取当前socket的error出错情况，内部使用getOption函数；
             */
            int getError();

            /**
             * @brief 输出信息到流中
             */
            std::ostream&  dump(std::ostream& os) const;
            
            /**
             * @brief 返回socket句柄
             */
            int getSocket() const { return m_sock; }

            /**
             * @brief 取消读
             * 调用IOManager的对应的cancel系列函数；
             */
            bool cancelRead();
            
            /**
             * @brief 取消写
             */
            bool cancelWrite();
            
            /**
             * @brief 取消accept
             */
            bool cancelAccept();
            
            /**
             * @brief 取消所有事件
             */
            bool cancelAll();

        private:
            
            /**
             * @brief 初始化socket
             * 调用setOption函数，设置当前套接字的属性，默认使用REUSEADDR属性
             * 如果是TCP的话，开启TCP_NODELAY属性，提升广域网传输效率；
             */
            void initSock();
            
            /**
             * @brief 创建socket
             * 调用socket()系统函数，并使用initSock()来进行初始化；
             */
            void newSock();
        
        private:
            /// socket句柄
            int m_sock;
            
            /// 协议簇
            int m_family;
            
            /// 类型
            int m_type;
            
            /// 协议
            int m_protocol;
            
            /// 是否连接
            bool m_isConnected;
            
            /// 本地地址
            Address::ptr m_localAddress;
            
            /// 远端地址
            Address::ptr m_remoteAddress;
    };

    std::ostream& operator<<(std::ostream& os, const Socket& sock);
} // namespace myhttp


#endif
