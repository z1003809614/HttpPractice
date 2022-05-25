#ifndef __MYHTTP_CONFIG_H__
#define __MYHTTP_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "log.h"


namespace myhttp{

    class ConfigVarBase{
        public:
            typedef std::shared_ptr<ConfigVarBase> ptr;

            ConfigVarBase(const std::string& name, const std::string& description="")
                : m_name(name)
                , m_description(description){
                    std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
                }

            virtual ~ConfigVarBase(){}

            const std::string& getName() const { return m_name;}
            const std::string& getDescription() const { return m_description;}
 
            virtual std::string toString() = 0;
            virtual bool fromString(const std::string& val) = 0;

        protected:
            std::string m_name;
            std::string m_description;
    };


    // F from_type, T to_type
    // 目前该模板只能支持基础类型的互相转换 -- 5/25；
    template<class F,class T>
    class LexicalCast{
        public:
            T operator()(const F& v){
                return boost::lexical_cast<T>(v);
            }
    };

    // 模板偏特化；
    template<class T>
    class LexicalCast<std::string, std::vector<T> >{
        public:
            std::vector<T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::vector<T> vec;
                std::stringstream ss;
                for(size_t i = 0; i< node.size(); ++i){
                    ss.str("");
                    ss << node[i];
                    vec.push_back(LexicalCast<std::string, T>()(ss.str()));
                }
                return vec ;
            }
    };

    template<class T>
    class LexicalCast<std::vector<T>, std::string>{
        public:
            std::string operator()(const std::vector<T>& v){
                YAML::Node node;
                for(auto& i : v){
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    // 为了支持复杂类的配置，需要序列化和反序列化的操作；-- 5/25;
    //FromStr: T operator() (const std::string&)
    //ToStr: std::string operator() (const T&)
    template<class T, class FromStr = LexicalCast<std::string, T>
                    , class ToStr = LexicalCast<T, std::string> >
    class ConfigVar : public ConfigVarBase{
        public:
            typedef std::shared_ptr<ConfigVar> ptr;
            ConfigVar(const std::string& name,const T& default_value, const std::string& description = "")
                :ConfigVarBase(name, description)
                ,m_val(default_value){
            }

            std::string toString() override {
                try{
                    //return boost::lexical_cast<std::string>(m_val);
                    return ToStr()(m_val);
                }catch(std::exception& e){
                    MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ConfigVar::toString exception"
                        << e.what() << " convert: " << typeid(m_val).name() << " to string";
                }
                return "";
            }

            bool fromString(const std::string& val) override{
                try
                {
                    // m_val = boost::lexical_cast<T>(val);
                    setValue(FromStr()(val));
                    return true;
                }
                catch(const std::exception& e)
                {
                   MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ConfigVar::toString exception"
                        << e.what() << " convert: " << typeid(m_val).name();
                }
                return false;
            }

            const T getValue() const {return m_val;}
            void setValue(const T& v) {m_val = v;}

        private:
            T m_val;
    };

    class Config{
        public:
            typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

            template<class T>
            static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                    const T& default_value, const std::string& description = "")
            {
                auto tmp =  Lookup<T>(name);
                if(tmp) {
                    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "Lookup name=" << name << " exists";
                    return tmp;
                }

                if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789")
                        != std::string::npos)
                {
                    MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "Lookup name invalid " << name;
                    throw std::invalid_argument(name);
                }

                typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
                s_datas[name] = v;
                return v;
            }
            
            template<class T>
            static typename ConfigVar<T>::ptr Lookup(const std::string& name){
                auto it = s_datas.find(name);

                if(it == s_datas.end()){
                    return nullptr;
                }

                return std::dynamic_pointer_cast<ConfigVar<T> > (it->second);
            }

            static void LoadFromYaml(const YAML::Node& root);

            static ConfigVarBase::ptr LookupBase(const std::string& name);

        private:
            static ConfigVarMap s_datas;
    };

   

}

#endif