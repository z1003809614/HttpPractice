#include "servlet.h"
#include "fnmatch.h"

namespace myhttp
{
    namespace http
    {
        //================================FunctionServlet======================================
        
        
        FunctionServlet::FunctionServlet(FunctionServlet::callback cb)
            :Servlet("FunctionServlet")
            ,m_cb(cb){}
                
        int32_t FunctionServlet::handle(myhttp::http::HttpRequest::ptr request,
                                        myhttp::http::HttpResponse::ptr response,
                                        myhttp::http::HttpSession::ptr session)
        {
            return m_cb(request, response, session);
        }
        


        //================================ServletDispatch======================================
        
        ServletDispatch::ServletDispatch()
            :Servlet("ServletDispatch"){
            m_default.reset(new NotFoundServlet());
        }

        int32_t ServletDispatch::handle(myhttp::http::HttpRequest::ptr request,
                                        myhttp::http::HttpResponse::ptr response,
                                        myhttp::http::HttpSession::ptr session) 
        {
            auto slt = getMatchedServlet(request->getPath());
            if(slt){
                slt->handle(request, response, session);
            }
            return 0;
        }


        void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt){
            RWMutexType::WriteLock lock(m_mutex);
            m_datas[uri] = slt;
        }

        void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::callback cb){
            RWMutexType::WriteLock lock(m_mutex);
            m_datas[uri].reset(new FunctionServlet(cb));
        }

        void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt){
            RWMutexType::WriteLock lock(m_mutex);
            for(auto it = m_globs.begin(); it != m_globs.end(); ++it){
                if(it->first == uri){
                    m_globs.erase(it);
                    break;
                }
            }
            m_globs.push_back(std::make_pair(uri, slt));
        }

        void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb){
            addGlobServlet(uri, FunctionServlet::ptr(new FunctionServlet(cb)));
        }

        void ServletDispatch::delServlet(const std::string& uri){
            RWMutexType::WriteLock lock(m_mutex);
            m_datas.erase(uri);
        }

        void ServletDispatch::delGolbServlet(const std::string& uri){
            RWMutexType::WriteLock lock(m_mutex);
            for(auto it = m_globs.begin(); it != m_globs.end(); ++it){
                if(it->first == uri){
                    m_globs.erase(it);
                    break;
                }
            }
        }

        Servlet::ptr ServletDispatch::getServlet(const std::string& uri){
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_datas.find(uri);
            return it == m_datas.end() ? nullptr : it->second;
        }

        Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri){
            RWMutexType::ReadLock lock(m_mutex);
            for(auto it = m_globs.begin(); it != m_globs.end(); ++it){
                if(it->first == uri){
                    return it->second;
                }
            }
            return nullptr;
        }

        Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri){
            RWMutexType::ReadLock lock(m_mutex);
            auto mit = m_datas.find(uri);
            if(mit != m_datas.end()){
                return mit->second;
            }
            for(auto it = m_globs.begin(); it != m_globs.end(); ++it){
                if(!fnmatch(it->first.c_str(), uri.c_str(), 0)){
                    return it->second;
                }
            }
            return m_default;
        }

        //========================NotFound=================================================
                
        NotFoundServlet::NotFoundServlet(const std::string& server_name)
            :Servlet("NotFoundServlet")
            ,m_serverName(server_name)
            {}

        int32_t NotFoundServlet::handle(myhttp::http::HttpRequest::ptr request,
                                       myhttp::http::HttpResponse::ptr response,
                                       myhttp::http::HttpSession::ptr session)
        {
            static const std::string& RSP_BODY = "<html><head><title>404 Not Found"
                "</title></head><body><h1>Not Found</h1><p>The requested URL /404/ was not found on this server.</p>"
                "<hr><address>myhttp/1.0,0 Port 80</address></body></html>";
            
            response->setStatus(myhttp::http::HttpStatus::NOT_FOUND);
            response->setHeader("Server", "myhttp/1.0.0");
            response->setHeader("Content-Type", "text/html");
            response->setBody(RSP_BODY);
            return 0;
        }
    } // namespace http
    
} // namespace myhttp
