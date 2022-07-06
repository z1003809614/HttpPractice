#include "../myhttp/http/http_parser.h"
#include "../myhttp/log.h"

static myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

const char test_request_data[] = "GET / HTTP/1.1\r\n"
                                 "Host: www.sylar.top\r\n"
                                 "Content-Length: 10\r\n\r\n"
                                 "1234567890";

void test(){
    myhttp::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    MYHTTP_LOG_INFO(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " conten_length=" << parser.getContentLength();

    tmp.resize(tmp.size()-s);
    MYHTTP_LOG_INFO(g_logger) << parser.getData()->toString();
    MYHTTP_LOG_INFO(g_logger) << tmp;
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                "Date: Wed, 06 Jul 2022 03:41:17 GMT\r\n"
                                "Server: Apache\r\n"
                                "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                "ETag: \"51-47cf7e6ee8400\"\r\n"
                                "Accept-Ranges: bytes\r\n"
                                "Content-Length: 81\r\n"
                                "Cache-Control: max-age=86400\r\n"
                                "Expires: Thu, 07 Jul 2022 03:41:17 GMT\r\n"
                                "Connection: Close\r\n"
                                "Content-Type: text/html\r\n\r\n"
                                "<html>\r\n"
                                "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                "</html>\r\n";


void test_response(){
    myhttp::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    MYHTTP_LOG_INFO(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " conten_length=" << parser.getContentLength();
    
    tmp.resize(tmp.size() - s);

    MYHTTP_LOG_INFO(g_logger) << parser.getData()->toString();
    MYHTTP_LOG_INFO(g_logger) << tmp;
}

int main(int argc, char** argv){
    test();
    test_response();
    return 0;
}