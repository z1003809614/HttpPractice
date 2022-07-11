/**
 * @file uri.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief 封装了uri(统一资源标识符)
 * @version 0.1
 * @date 2022-07-11
 * 
 * uri封装类，提供解析字符串uri功能；
 * 其解析方法使用的ragel的有限状态机来完成的；
 * ragel的使用和解析方法需要进一步了解；
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __MYHTTP_URI_H__
#define __MYHTTP_URI_H__

#include <memory>
#include <string>
#include <stdint.h>
#include "address.h"

namespace myhttp
{

    /*
     foo://user@sylar.com:8042/over/there?name=ferret#nose
     \_/   \_________________/ \________/ \_________/ \__/
      |             |              |           |        |
    scheme      authority         path       query   fragment
    */

    class Uri
    {
        public:
            /// 只能指针定义
            typedef std::shared_ptr<Uri> ptr;

            /**
             * @brief 解析字符串uri自动生成uri对象
             * @param[in] uri：输入字符串 
             * @return Uri::ptr ：成功返回uri对象的智能指针，失败返回nullptr
             */
            static Uri::ptr Create(const std::string& uri);

            /**
             * @brief Uri对象构造函数，内部只初始化了m_port成员变量;
             */
            Uri();

            /**
             * @brief 返回m_scheme
             */
            const std::string& getScheme() const { return m_scheme; }
            
            /**
             * @brief 返回m_userinfo
             */
            const std::string& getUserinfo() const { return m_userinfo; }
            
            /**
             * @brief 返回m_host
             */
            const std::string& getHost() const { return m_host; }
            
            /**
             * @brief 返回m_query
             */
            const std::string& getQuery() const { return m_query; }
            
            /**
             * @brief 返回m_fragment
             */
            const std::string& getFragment() const { return m_fragment; }
            
            /**
             * @brief 返回m_port
             * 实现的地方与isDefaultPort有重复部分，可以优化
             */
            int32_t getPort() const;
           
            /**
             * @brief 返回m_path
             * m_path为空的时候，会返回"/";
             * 这里应该可以在构造的时候就默认初始化为根目录
             * 省去了这里的判读；
             */
            const std::string& getPath() const;

            /**
             * @brief 设置m_scheme
             * @param v scheme
             */
            void setScheme(const std::string& v)  { m_scheme = v; }
            
            /**
             * @brief 设置m_userinfo
             * @param v userinfo
             */
            void setUserinfo(const std::string& v)  { m_userinfo = v; }
            
            /**
             * @brief 设置m_host
             * @param v host
             */
            void setHost(const std::string& v)  { m_host = v; }
            
            /**
             * @brief 设置m_path
             * @param v path
             */
            void setPath(const std::string& v)  { m_path = v; }
            
            /**
             * @brief 设置m_query
             * @param v query
             */
            void setQuery(const std::string& v)  { m_query = v; }
            
            /**
             * @brief 设置m_fragment
             * @param v fragment
             */
            void setFragment(const std::string& v)  { m_fragment = v; }
            
            /**
             * @brief 设置m_port
             * @param v port
             */
            void setPort(int32_t v) { m_port = v; }

            /**
             * @brief 将当前uri转换为string序列并存储到输出流
             *      这个api设计,感觉也有点问题，输入参数是引用的，
             *      可以直接改变其状态，应该不需要再进行返回；
             * @param os 输出流 
             * @return std::ostream& 输出流
             */
            std::ostream& dump(std::ostream& os) const;
            
            /**
             * @brief 调用dump，输出序列化字符串；
             * @return std::string 
             */
            std::string toString() const;

            /**
             * @brief 获取Address
             *      uri可以具化为url和urn，现在url占多数，url中authority表现为host，
             *      故可以将其转换为address对象，方便后续socket的连接使用；
             * @return Address::ptr 
             */
            Address::ptr createAddress() const; 

        private:
            
            /**
             * @brief 是否为默认端口，默认端口 = 0；
             */
            bool isDefaultPort() const;

        private:
            
            /// scheme,可以理解为协议
            std::string m_scheme;

            /// 用户信息,这里应该是urn会产生类似的信息；
            std::string m_userinfo;
            
            /// host,可以理解为域名
            std::string m_host;

            /// 端口号
            int32_t m_port;

            /// 路径信息，即在host下的具体路径
            std::string m_path;

            /// 查询参数
            std::string m_query;

            /// fragment
            std::string m_fragment;
    };
    
} // namespace myhttp




#endif
