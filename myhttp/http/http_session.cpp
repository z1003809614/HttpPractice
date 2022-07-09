#include "http_session.h"
#include "http_parser.h"
#include "../log.h"

namespace myhttp
{
    namespace http
    {

        static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system"); 

        HttpSession::HttpSession(Socket::ptr sock, bool owner)
            :SocketStream(sock, owner){
        }

        HttpRequest::ptr HttpSession::recvRequest(){
            HttpRequestParser::ptr parser(new HttpRequestParser);
            
            /**
             * @brief 测试 小buff_size的解析情况
             * 正常：
             *      超过buff_size的不能够解析，没超过的能正常解析；
             * 目前测试情况： 
             *      直接解析出错，无法获得request对象 -- 2022/7/8/15:00；
             * 上面出现测试错误的原因：
             *      当出现无法进行完整解析的情况的时候，http_parser.cpp中on_request_http_field()
             *      会设置m_error = 1002, 触发hasError()函数，return nullptr;
             */
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            // uint64_t buff_size = 100; 
            
            std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){
                delete[] ptr;
            });

            char* data = buffer.get();
            
            
            /**
             * @brief 表示data中有多少数据没有被解析的位置；
             * httpHeader的解析，是以key-value形式来解析，buff_size一般是能够完全容纳一个键值对的；
             * 但是键值对的长度是不定的，每次进行read是会无差别读取数据，一般都会将data用完；
             * 所以引入了offset变量，把data中没有进行解析的数据进行记录；
             */
            int offset = 0; 

            do{
                /**
                 * @brief 从socket中，读取buff_size-offset个字节数据到data + offset位置中
                 * 在执行execute的时候，会将没有完成解析的数据，通过memmove()函数进行搬移，
                 * 所以需要使用到offset；
                 */
                int len = read(data + offset, buff_size - offset);
                if(len <= 0){
                    return nullptr;
                }

                len += offset;

                size_t nparser = parser->execute(data, len);
                if(parser->hasError()){
                    MYHTTP_LOG_ERROR(g_logger) << "parser find error";
                    return nullptr;
                }

                offset = len - nparser;

                if(offset == (int)buff_size){
                    return nullptr;
                }

                if(parser->isFinished()){
                    break;
                }

            }while(true);

            int64_t length = parser->getContentLength();
            if(length > 0){
                std::string body;
                body.reserve(length);
                
                if(length >= offset){
                    body.append(data, offset);
                }else{
                    body.append(data, length);
                }
                length -= offset;
                if(length > 0){
                    if(readFixSize(&body[body.size()], length) <= 0 ){
                        return nullptr;
                    } 
                }
                parser->getData()->setBody(body);
            }
            return parser->getData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp){
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    } // namespace http
    
} // namespace myhttp
