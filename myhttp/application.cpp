#include "application.h"
#include "env.h"
#include "config.h"
#include "daemon.h"
#include "iomanager.h"
#include "address.h"


namespace myhttp
{

    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    // 工作目录配置项
    static myhttp::ConfigVar<std::string>::ptr g_server_work_path =
        myhttp::Config::Lookup("server.work_path"
                ,std::string("/home/apps/work/myhttp")
                , "server work path");
    
    // 进程pidfile的名称配置项 
    static myhttp::ConfigVar<std::string>::ptr g_server_pid_file =
        myhttp::Config::Lookup("server.pid_file"
                ,std::string("myhttp.pid")
                , "server pid file");

    // 保存http配置项
    struct HttpServerConf{
        // 服务器监听地址列表
        std::vector<std::string> address;
        // 是否长连接
        int keepalive = 0;
        // 超时时间
        int timeout = 1000 * 2 * 60;
        // 服务器名称
        std::string name;

        bool isValid() const{
            return !address.empty();
        }

        bool operator==(const HttpServerConf& oth) const{
            return address == oth.address
                && keepalive == oth.keepalive
                && timeout == oth.timeout
                && name == oth.name;
        }
    };

    // YAML string -> HttpServerConf
    template<>
    class LexicalCast<std::string, HttpServerConf> {
        public:
            HttpServerConf operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                HttpServerConf conf;
                conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
                conf.timeout = node["timeout"].as<int>(conf.timeout);
                conf.name = node["name"].as<std::string>(conf.name);
                if(node["address"].IsDefined()){
                    for(size_t i = 0; i < node["address"].size(); ++i){
                        conf.address.push_back(node["address"][i].as<std::string>());
                    }
                }
                return conf;
            }
    };

    //  HttpServerConf -> YAML string
    template<>
    class LexicalCast<HttpServerConf, std::string> {
        public:
            std::string operator()(const HttpServerConf& conf){
                YAML::Node node;
                node["name"] = conf.name;
                node["keepalive"] = conf.keepalive;
                node["timeout"] = conf.timeout;
                for(auto& i : conf.address){
                    node["address"].push_back(i);
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    // server配置项
    static myhttp::ConfigVar<std::vector<HttpServerConf> >::ptr g_http_servers_conf = 
        myhttp::Config::Lookup("http_servers", std::vector<HttpServerConf>(), "http server config");

    // 静态成员变量，类外初始化
    Application* Application::s_instance = nullptr;

    // 将当前对象保存在this中；
    Application::Application(){
        s_instance = this;
    }

    bool Application::init(int argc, char** argv){
        
        m_argc = argc;
        m_argv = argv;
        myhttp::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
        myhttp::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
        myhttp::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
        myhttp::EnvMgr::GetInstance()->addHelp("p", "print help");

        // EnvMgr->init(),可以将字符串的参数以key-value的形式保存到env的成员变量中  
        if(!myhttp::EnvMgr::GetInstance()->init(argc,argv)){
            myhttp::EnvMgr::GetInstance()->printHelp();
            return false;
        }

        if(myhttp::EnvMgr::GetInstance()->has("p")){
            myhttp::EnvMgr::GetInstance()->printHelp();
            return false;
        }

        int run_type = 0;
        if(myhttp::EnvMgr::GetInstance()->has("s")){
            run_type = 1;
        }

        if(myhttp::EnvMgr::GetInstance()->has("d")){
            run_type = 2;
        }

        if(run_type == 0){
            myhttp::EnvMgr::GetInstance()->printHelp();
            return false;
        }

        // pidfile的完整路径信息；
        std::string pidfile = g_server_work_path->getValue() + 
                                "/" + g_server_pid_file->getValue();
        
        // 判断pidfile内部保存的是pid号进程是否正在执行；
        if(myhttp::FSUtil::IsRunningPidfile(pidfile)){
            MYHTTP_LOG_ERROR(g_logger) << "server is running: " << pidfile;
            return false;
        }

        // 是否知道配置文件目录，没有就执行默认的conf目录，并形成绝对路径
        std::string conf_path = myhttp::EnvMgr::GetInstance()->getAbsolutePath(
                myhttp::EnvMgr::GetInstance()->get("c", "conf")
        );

        MYHTTP_LOG_INFO(g_logger) << "load conf path:" << conf_path;
        
        // 加载配置文件
        myhttp::Config::LoadFromConfDir(conf_path);

        // 构建工作目录
        if(!myhttp::FSUtil::Mkdir(g_server_work_path->getValue())){
            MYHTTP_LOG_FATAL(g_logger) << "create work path=" << g_server_work_path->getValue()
                <<  " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }

        return true;
    }

    bool Application::run(){
        bool is_deamon = myhttp::EnvMgr::GetInstance()->has("d");
        return start_daemon(m_argc, m_argv, 
                    std::bind(&Application::main, this, std::placeholders::_1,std::placeholders::_2), is_deamon);
    }

    int Application::main(int argc, char** argv){
        
        std::string pidfile = g_server_work_path->getValue() + 
                                "/" + g_server_pid_file->getValue();
        
        std::ofstream ofs(pidfile);
        
        if(!ofs){
            MYHTTP_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        // 写入当前正在执行的进程号
        ofs << getpid();

        auto http_confs = g_http_servers_conf->getValue();
        
        for(auto& i : http_confs){
            MYHTTP_LOG_INFO(g_logger) << LexicalCast<HttpServerConf, std::string>()(i);
        }


        myhttp::IOManager iom(1);
        iom.schedule(std::bind(&Application::run_fiber, this));
        iom.stop();

        return 0;
    }

    int Application::run_fiber(){
    
        auto http_confs = g_http_servers_conf->getValue();

        // 根据server配置项进行服务器启动    
        for(auto& i : http_confs){
            
            MYHTTP_LOG_INFO(g_logger) << LexicalCast<HttpServerConf, std::string>()(i);

            std::vector<Address::ptr> address;
            // 判断配置项中的地址信息是否可用；
            for(auto& a : i.address){
                // 目前只考虑了ipv4的地址格式
                size_t pos = a.find(":");
                
                if(pos == std::string::npos){
                    MYHTTP_LOG_ERROR(g_logger) << "invalid address: " << a;
                    continue;
                }
                
                auto addr = myhttp::Address::LookupAny(a);
                
                if(addr){
                    address.push_back(addr);
                    continue;
                }
                
                // 保存该ip对应的网卡信息；
                std::vector<std::pair<Address::ptr,uint32_t> > result;

                if(myhttp::Address::GetInterfaceAddresses(result, a.substr(0,pos))){
                    MYHTTP_LOG_ERROR(g_logger) << "invalid address: " << a;
                    continue;
                }
                
                // 将该网卡的所有iP都添加到监听地址数据中；
                for(auto& x : result){
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if(ipaddr){
                        ipaddr->setPort(atoi(a.substr(pos+1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
            }

            // server内部就使用了iomanager，即用到了协程
            myhttp::http::HttpServer::ptr server(new myhttp::http::HttpServer(i.keepalive));
            
            std::vector<Address::ptr> fails;
            
            if(!server->bind(address, fails)){
                for(auto& x : fails){
                    MYHTTP_LOG_ERROR(g_logger) << "bind address fail:"
                        << x->toString();
                }
                _exit(0);
            }
            
            if(!i.name.empty()){
                server->setName(i.name);
            }
            
            server->start();
            
            m_httpservers.push_back(server);
        }

        return 0;
    }


} // namespace myhttp
