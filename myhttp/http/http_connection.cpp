#include "http_connection.h"
#include "http_parser.h"
#include "../log.h"

namespace myhttp
{
    namespace http
    {

        static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system"); 

        HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
            :SocketStream(sock, owner){
        }

        HttpResponse::ptr HttpConnection::recvResponse(){
            HttpResponseParser::ptr parser(new HttpResponseParser);
            
            uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();
            
            std::shared_ptr<char> buffer(new char[buff_size + 1], [](char* ptr){
                delete[] ptr;
            });

            char* data = buffer.get();
            
            int offset = 0; 

            // 解析haeder部分
            do{
                int len = read(data + offset, buff_size - offset);
                if(len <= 0){
                    MYHTTP_LOG_ERROR(g_logger) << "read length <= 0";
                    return nullptr;
                }

                len += offset;
                
                data[len] = '\0';
                
                size_t nparser = parser->execute(data, len, false);
                if(parser->hasError()){
                    MYHTTP_LOG_ERROR(g_logger) << "parser has error";
                    return nullptr;
                }

                offset = len - nparser;

                if(offset == (int)buff_size){
                    MYHTTP_LOG_ERROR(g_logger) << "offset == buff_size";
                    return nullptr;
                }

                if(parser->isFinished()){
                    break;
                }
            }while(true);

            // 解析body部分,如果头部解析得到body使用chunck模式，则需要额外进行处理；
            // chunck编码：[content-length]\r\n[data]\r\n;
            auto& client_parser = parser->getParser();
            if(client_parser.chunked){
                std::string body;
                int len = offset;
                do{ // 每次执行读取一行chunck；
                    do{ // 每次循环结束能够解析一个chunck的头部信息，即content-length;
                        int rt = read(data + len, buff_size - len);
                        if(rt <= 0){
                            return nullptr;
                        }
                        len += rt;
                        data[len] = '\0';
                        size_t nparse = parser->execute(data, len, true);
                        if(parser->hasError()){
                            return nullptr;
                        }
                        len -= nparse;
                        if(len == (int)buff_size){
                            return nullptr;
                        }
                    }while(!parser->isFinished());
                    
                    len -= 2; // chunck-data会给存储的原始数据中额外加上一个\r\n，所有这里存储body的时候需要手动去掉；

                    if(client_parser.content_len <= len){ // body长度小于len时，表示body数据已经读取到data中；

                        body.append(data, client_parser.content_len);
                        
                        memmove(data, data + client_parser.content_len, len - client_parser.content_len);
                        
                        len -= client_parser.content_len;

                    }else{  // body数据还未读取到data中；
                        body.append(data, len);
                        int left = client_parser.content_len - len;
                        while(left > 0){
                            int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                            if(rt <= 0){
                                return nullptr;
                            }
                            body.append(data, rt);
                            left -= rt;
                        }
                        len = 0;
                    }
                }while(!client_parser.chunks_done);

                parser->getData()->setBody(body);

            }else{
                int64_t length = parser->getContentLength();
                if(length > 0){
                    std::string body;
                    body.resize(length);
                    
                    int len = 0;
                    if(length >= offset){
                        memcpy(&body[0], data, offset);
                        len = offset;
                    }else{
                        memcpy(&body[0], data, length);
                        len = length;
                    }
                    length -= offset;
                    if(length > 0){
                        if(readFixSize(&body[len], length) <= 0 ){
                            MYHTTP_LOG_ERROR(g_logger) << "readFixSize <= 0";
                            return nullptr;
                        } 
                    }
                    parser->getData()->setBody(body);
                }
            }
            return parser->getData();
        }

        int HttpConnection::sendRequest(HttpRequest::ptr req){
            std::stringstream ss;
            ss << *req;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    } // namespace http
    
} // namespace myhttp
