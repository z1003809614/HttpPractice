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
            
                size_t execute(const char* data, size_t len, size_t off);
                int isFinished() const;
                int hasError() const; 

                HttpRequest::ptr getData() const { return m_data; }
            private:
                http_parser m_parser;
                HttpRequest::ptr m_data;
                int m_error;
        };

        class HttpResponseParser{
            public:
                typedef std::shared_ptr<HttpResponseParser> ptr;
                HttpResponseParser();
        
                size_t execute(const char* data, size_t len, size_t off);
                int isFinished() const;
                int hasError() const; 

                HttpResponse::ptr getData() const { return m_data; }
            private:
                httpclient_parser m_parser;
                HttpResponse::ptr m_data;
                int m_error;
        };
    } // namespace htt
    
} // namespace myhttp

#endif
