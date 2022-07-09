#ifndef __MYHTTP_HTTP_PAESER_H__
#define __MYHTTP_HTTP_PAESER_H__


#include "httpclient_parser.h"
#include "http11_parser.h"
#include "http.h"

namespace myhttp
{
    namespace http
    {
        class HttpRequestParser{
            public:
                typedef std::shared_ptr<HttpRequestParser> ptr;
                HttpRequestParser();

                // 对data执行len长度的http解析；
                size_t execute(char* data, size_t len);
                // 当前解析是否完成；
                int isFinished();
                // 解析过程是否出错；
                int hasError(); 

                // 获取解析后经过封装的data对象；
                HttpRequest::ptr getData() const { return m_data; }
                // 设定自定义error信息
                void setError(int v) { m_error = v; }

                // 获取报文中body的长度；
                uint64_t getContentLength();
            
                const http_parser& getParser() const { return m_parser; }
            public:
                static uint64_t GetHttpRequestBufferSize();
                static uint64_t GetHttpRequestMaxBodySize();
            private:
                http_parser m_parser;
                HttpRequest::ptr m_data;
                // 1000 : invalid method;
                // 1001 : invalid version;
                // 1002 : invalid field;
                int m_error;
        };

        class HttpResponseParser{
            public:
                typedef std::shared_ptr<HttpResponseParser> ptr;
                HttpResponseParser();
        
                size_t execute(char* data, size_t len, bool chunck);
                int isFinished();
                int hasError(); 

                HttpResponse::ptr getData() const { return m_data; }
                void setError(int v) { m_error = v; }

                uint64_t getContentLength();

                const httpclient_parser& getParser() const { return m_parser; }
            public:
                static uint64_t GetHttpResponseBufferSize();
                static uint64_t GetHttpResponseMaxBodySize();
             
            private:
                httpclient_parser m_parser;
                HttpResponse::ptr m_data;
                
                int m_error;
        };
    } // namespace htt
    
} // namespace myhttp

#endif
