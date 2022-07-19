/**
 * @file tcp_server.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief TCP服务器的封装
 * @version 0.1
 * @date 2022-07-17
 * @copyright Copyright (c) 2022
 */
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
            
            /**
             * @brief 构造函数
             * @param[in] worker socket客户端工作的协程调度器
             * @param[in] accept_worker 服务器socket执行接收socket连接的协程调度器
             */
            TcpServer(myhttp::IOManager* woker = myhttp::IOManager::GetThis(),
                    myhttp::IOManager* accept_worker = myhttp::IOManager::GetThis());
            
            /**
             * @brief 析构函数
             */
            virtual ~TcpServer(){}
            
            
            /**
             * @brief 绑定地址
             *      地址绑定(bind + listen)(内部直接生成一一对应的socket)
             * @return 返回是否绑定成功
             */
            virtual bool bind(myhttp::Address::ptr addr);
            
            /**
             * @brief 绑定地址数组
             * @param[in] addrs 需要绑定的地址数组
             * @param[out] fails 绑定失败的地址
             * @return 是否绑定成功
             */
            virtual bool bind(const std::vector<Address::ptr>& addrs, 
                              std::vector<Address::ptr>& fails);
            
            /**
             * @brief 启动服务
             * @pre 需要bind成功后执行
             */
            virtual bool start();
            
            /**
             * @brief 停止服务
             */
            virtual void stop();

            /**
             * @brief 返回读取超时时间(毫秒)
             */
            uint64_t getRecvTimeout() const { return m_recvTimeout; }
            
            /**
             * @brief 返回服务器名称
             */
            std::string getName() const { return m_name; }
            
            /**
             * @brief 设置读取超时时间(毫秒)
             */
            void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
            
            /**
             * @brief 设置服务器名称
             */
            void setName(const std::string& v) { m_name = v; }

             /**
             * @brief 是否停止
             */
            bool isStop() const { return m_isStop; }
        
        protected:

            /**
             * @brief 处理新连接的Socket类
             */
            virtual void handleClient(Socket::ptr client);
            
            /**
             * @brief 开始接受连接
             * 对listen的sock的请求队列调用accept
             */
            virtual void startAccept(Socket::ptr sock);
        
        private:
            
            /// 监听Socket数组
            std::vector<Socket::ptr> m_socks;
            
            /// 新连接的Socket工作的调度器
            IOManager* m_worker;
           
            /// 服务器Socket接收连接的调度器
            IOManager* m_acceptWorker;
            
            /// 接收超时时间(毫秒)
            uint64_t m_recvTimeout;
            
            /// 服务器名称
            std::string m_name;
            
            /// 服务是否停止
            bool m_isStop;
    
    };
} // namespace myhttp



#endif