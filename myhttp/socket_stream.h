#ifndef __MYHTTP_SOCKET_STREAM_H__
#define __MYHTTP_SOCKET_STREAM_H__

#include "stream.h"
#include "socket.h"

namespace myhttp
{
    class SocketStream : public Stream{
        public:
            typedef std::shared_ptr<SocketStream> ptr;
            // owner 是否全权管理(析构的时候是否辅助关闭句柄(fd))
            SocketStream(Socket::ptr sock, bool owner = true); 
            ~SocketStream();

            virtual int read(void* buffer, size_t length) override;
            virtual int read(ByteArray::ptr ba, size_t length) override;

            virtual int write(const void* buffer, size_t length) override;
            virtual int write(ByteArray::ptr ba, size_t length) override;
            virtual void close() override;
            
            Socket::ptr getSocket() const { return m_socket; }

            bool isConnected() const;
        protected:
            Socket::ptr m_socket;
            bool m_owner;
    };
} // namespace myhttp



#endif