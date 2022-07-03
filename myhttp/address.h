#ifndef __MYHTTP_ADDRESS_H__
#define __MYHTTP_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <map>

namespace myhttp
{   
    class IPAddress;
    class Address{
        public:

            typedef std::shared_ptr<Address> ptr;
            static Address::ptr Create(const sockaddr* addr, socklen_t addrlen); 

            /**
             * @brief 根据域名查找地址信息（不一定是IP族）
             * 
             * @param result ip地址信息，用于存储结果
             * @param host 域名信息
             * @param family 协议族
             * @param type ？
             * @param protocol 协议 
             * @return true 
             * @return false 
             */
            static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
                    int family = AF_UNSPEC, int type = 0, int protocol = 0);
            
            /**
             * @brief 根据域名返回任意一个对应的地址信息
             * 
             * @param host 
             * @param family 
             * @param type 
             * @param protocol 
             * @return Address::ptr 
             */
            static Address::ptr LookupAny(const std::string& host,
                    int family = AF_UNSPEC, int type = 0, int protocol = 0);
            
            /**
             * @brief 根据域名返回任意一个IP地址信息
             * 
             * @param host 
             * @param family 
             * @param type 
             * @param protocol 
             * @return IPAddress::ptr 
             */
            static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
                    int family = AF_UNSPEC, int type = 0, int protocol = 0);
            
            /**
             * @brief Get the Interface Addresses object 获得family协议族的网卡信息
             * @param result 
             * @param family 协议族
             * @return true 
             * @return false 
             */
            static bool GetInterfaceAddresses(std::multimap<std::string, 
                            std::pair<Address::ptr, uint32_t> >& result,
                            int family = AF_UNSPEC);
            
            /**
             * @brief 根据不同的网卡信息返回对应的地址信息；
             * @param result 
             * @param iface 
             * @param family 
             * @return true 
             * @return false 
             */
            static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result
                            ,const std::string& iface, int family = AF_UNSPEC);
        

            virtual ~Address(){}

            /**
             * @brief Get the Family
             * 
             * @return int 
             */
            int getFamily() const;

            virtual const sockaddr* getAddr() const = 0;
            virtual socklen_t getAddrLen() const = 0;

            // 当有函数未定义的时候，会出现link错误(address未定义)
            virtual std::ostream& insert(std::ostream& os) const = 0;
            std::string toString();

            bool operator<(const Address& rhs) const;
            bool operator==(const Address& rhs) const;
            bool operator!=(const Address& rhs) const;
    };

    class IPAddress : public Address{
        public:
            typedef std::shared_ptr<IPAddress> ptr;

            static IPAddress::ptr Create(const char* address, uint16_t port = 0);

            virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
            virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
            virtual IPAddress::ptr subnetMast(uint32_t prefix_len) = 0;
            
            virtual uint16_t getPort() const = 0;
            virtual void setPort(uint16_t v) = 0;
    };

    class IPv4Address : public IPAddress{
        public:
            typedef std::shared_ptr<IPv4Address> ptr;

            static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

            IPv4Address(const sockaddr_in& address);
            IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

            const sockaddr* getAddr() const override;
            socklen_t getAddrLen() const override;
            std::ostream& insert(std::ostream& os) const override;

            IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
            IPAddress::ptr networdAddress(uint32_t prefix_len) override;
            IPAddress::ptr subnetMast(uint32_t prefix_len) override;

            uint16_t getPort() const override;
            void setPort(uint16_t v) override;
        
        private:
            sockaddr_in m_addr;
    };

    class IPv6Address : public IPAddress{
        public:
            typedef std::shared_ptr<IPv6Address> ptr;
            static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

            IPv6Address();
            IPv6Address(const sockaddr_in6& address);
            IPv6Address(const uint8_t address[16], uint16_t port);

            const sockaddr* getAddr() const override;
            socklen_t getAddrLen() const override;
            std::ostream& insert(std::ostream& os) const override;

            IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
            IPAddress::ptr networdAddress(uint32_t prefix_len) override;
            IPAddress::ptr subnetMast(uint32_t prefix_len) override;

            uint16_t getPort() const override;
            void setPort(uint16_t v) override;
        
        private:
            sockaddr_in6 m_addr;
    };

    class UnixAddress : public Address{
        public:
            typedef std::shared_ptr<UnixAddress> ptr;
            UnixAddress();
            UnixAddress(const std::string& path);

            const sockaddr* getAddr() const override;
            socklen_t getAddrLen() const override;
            void setAddrLen(uint32_t v);

            std::ostream& insert(std::ostream& os) const override;
        private:
            sockaddr_un m_addr;
            socklen_t m_length;
    };

    class UnknownAddress : public Address{
        public:
            typedef std::shared_ptr<UnknownAddress> ptr;
            
            UnknownAddress(int family);
            UnknownAddress(const sockaddr& addr);

            const sockaddr* getAddr() const override;
            socklen_t getAddrLen() const override;
            std::ostream& insert(std::ostream& os) const override;
        private:
            sockaddr m_addr;
    };

    
} // namespace myhttp




#endif