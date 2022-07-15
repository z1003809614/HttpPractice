#ifndef __MYHTTP_HTTP_SERVLET_H__
#define __MYHTTP_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../thread.h"

namespace myhttp
{
    namespace http
    {   
        /**
        * @brief Servlet抽象基类
        * handle为servlet的执行逻辑函数；
        */
        class Servlet{
            public:
                typedef std::shared_ptr<Servlet> ptr;

                Servlet(const std::string& name)
                    :m_name(name){}

                virtual ~Servlet(){}
                virtual int32_t handle(myhttp::http::HttpRequest::ptr request,
                                myhttp::http::HttpResponse::ptr response,
                                myhttp::http::HttpSession::ptr session) = 0;

                const std::string& getName() const { return m_name; }
            protected:
                std::string m_name;
        };

        /**
         * @brief 可以定义回调函数来实现servlet的行为函数；
         */
        class FunctionServlet : public Servlet{
            public:
                typedef std::shared_ptr<FunctionServlet> ptr;
                typedef std::function<int32_t (myhttp::http::HttpRequest::ptr request,
                                myhttp::http::HttpResponse::ptr response,
                                myhttp::http::HttpSession::ptr session)> callback;
                
                FunctionServlet(callback cb);
                
                virtual int32_t handle(myhttp::http::HttpRequest::ptr request,
                                myhttp::http::HttpResponse::ptr response,
                                myhttp::http::HttpSession::ptr session) override;

            private:
                callback m_cb;
        };

        /**
         * @brief servlet分发类，相当于一个magnager的角色；
         * 用于管理和协调不同的servlet
         */
        class ServletDispatch : public Servlet
        {
            public:
                typedef std::shared_ptr<ServletDispatch> ptr;
                typedef myhttp::RWMutex RWMutexType;

                ServletDispatch();

                virtual int32_t handle(myhttp::http::HttpRequest::ptr request,
                                myhttp::http::HttpResponse::ptr response,
                                myhttp::http::HttpSession::ptr session) override;


                void addServlet(const std::string& uri, Servlet::ptr slt);
                void addServlet(const std::string& uri, FunctionServlet::callback cb);
                void addGlobServlet(const std::string& uri, Servlet::ptr slt);
                void addGlobServlet(const std::string& uri, FunctionServlet::callback);

                void delServlet(const std::string& uri);
                void delGolbServlet(const std::string& uri);

                Servlet::ptr getServlet(const std::string& uri);
                Servlet::ptr getGlobServlet(const std::string& uri);

                Servlet::ptr getDefault() const { return m_default; }
                void setDefault(Servlet::ptr v) { m_default = v; }

                Servlet::ptr getMatchedServlet(const std::string& uri);
            private:
                //uri(/myhttp/xxx) -> servlet
                std::unordered_map<std::string, Servlet::ptr> m_datas;
                
                //uri(/myhttp/*) -> servlet
                std::vector<std::pair<std::string, Servlet::ptr> > m_globs;

                //默认servlet, 所有路径都没匹配到时使用
                Servlet::ptr m_default;

                RWMutexType m_mutex;
        };


        class NotFoundServlet : public Servlet{
            public:
                typedef std::shared_ptr<NotFoundServlet> ptr;
                
                
                NotFoundServlet(const std::string& server_name = "myhttp");
                
                virtual int32_t handle(myhttp::http::HttpRequest::ptr request,
                                       myhttp::http::HttpResponse::ptr response,
                                       myhttp::http::HttpSession::ptr session) override;
                
                
            
            private:
                std::string m_serverName;

        };
         
    } // namespace http
    
} // namespace myhtto



#endif