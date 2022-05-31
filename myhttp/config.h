#ifndef __MYHTTP_CONFIG_H__
#define __MYHTTP_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
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

            virtual std::string getTypeName() const = 0;

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

    template<class T>
    class LexicalCast<std::string, std::list<T> >{
        public:
            std::list<T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::list<T> List;
                std::stringstream ss;
                for(size_t i = 0; i< node.size(); ++i){
                    ss.str("");
                    ss << node[i];
                    List.push_back(LexicalCast<std::string, T>()(ss.str()));
                }
                return List;
            }
    };
    template<class T>
    class LexicalCast<std::list<T>, std::string>{
        public:
            std::string operator()(const std::list<T>& v){
                YAML::Node node;
                for(auto& i : v){
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };


    template<class T>
    class LexicalCast<std::string, std::set<T> >{
        public:
            std::set<T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::set<T> seT;
                std::stringstream ss;
                for(size_t i = 0; i< node.size(); ++i){
                    ss.str("");
                    ss << node[i];
                    seT.insert(LexicalCast<std::string, T>()(ss.str()));
                }
                return seT;
            }
    };
    template<class T>
    class LexicalCast<std::set<T>, std::string>{
        public:
            std::string operator()(const std::set<T>& v){
                YAML::Node node;
                for(auto& i : v){
                    // 这里使用Load函数，是为了输出字符串的时候，与yam格式一致；-- 5/26；
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    template<class T>
    class LexicalCast<std::string, std::unordered_set<T> >{
        public:
            std::unordered_set<T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::unordered_set<T> unord_set;
                std::stringstream ss;
                for(size_t i = 0; i< node.size(); ++i){
                    ss.str("");
                    ss << node[i];
                    unord_set.insert(LexicalCast<std::string, T>()(ss.str()));
                }
                return unord_set;
            }
    };
    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string>{
        public:
            std::string operator()(const std::unordered_set<T>& v){
                YAML::Node node;
                for(auto& i : v){
                    // 这里使用Load函数，是为了输出字符串的时候，与yam格式一致；-- 5/26；
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    template<class T>
    class LexicalCast<std::string, std::map<std::string, T> >{
        public:
            std::map<std::string, T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::map<std::string, T> mp;
                std::stringstream ss; // 这里ss是用来做转换的；
                for(auto it = node.begin(); it!=node.end(); ++it){
                    ss.str("");
                    ss << it->second;
                    mp.insert(std::make_pair(it->first.Scalar(),
                                             LexicalCast<std::string, T>()(ss.str())));
                }
                return mp;
            }
    };
    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string>{
        public:
            std::string operator()(const std::map<std::string, T>& v){
                YAML::Node node;
                for(auto& i : v){
                    // 这里使用Load函数，是为了输出字符串的时候，与yam格式一致；-- 5/26；
                    node[i.first] = (YAML::Load(LexicalCast<T, std::string>()(i.second)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };


    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T> >{
        public:
            std::unordered_map<std::string, T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                std::unordered_map<std::string, T> ump;
                std::stringstream ss; // 这里ss是用来做转换的；
                for(auto it = node.begin(); it!=node.end(); ++it){
                    ss.str("");
                    ss << it->second;
                    ump.insert(std::make_pair(it->first.Scalar(),
                                             LexicalCast<std::string, T>()(ss.str())));
                }
                return ump;
            }
    };
    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>{
        public:
            std::string operator()(const std::unordered_map<std::string, T>& v){
                YAML::Node node;
                for(auto& i : v){
                    // 这里使用Load函数，是为了输出字符串的时候，与yam格式一致；-- 5/26；
                    node[i.first] = (YAML::Load(LexicalCast<T, std::string>()(i.second)));
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
            typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

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
                   MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ConfigVar::fromString exception"
                        << e.what() << " convert: " << typeid(m_val).name();
                }
                return false;
            }

            const T getValue() const {return m_val;}
            
            void setValue(const T& v) {
                if(v == m_val){
                    return;
                }
                // m_cbs指向回调函数；
                for(auto& i : m_cbs){
                    i.second(m_val, v);
                }
                m_val = v;
            }

            std::string getTypeName() const override { return typeid(T).name(); }

            void addListener(uint64_t key, on_change_cb cb){
                m_cbs[key] = cb;
            }

            void delListener(uint64_t key){
                m_cbs.erase(key);
            }

            on_change_cb getListener(uint64_t key){
                auto it = m_cbs.find(key);
                return it == m_cbs.end() ? nullptr : it->second;
            }

            void clearListener() {
                m_cbs.clear();
            }

        private:
            T m_val;
            // 变更回调函数组，uint64_t key ,要求唯一， 一般可以用hash;
            std::map<uint64_t, on_change_cb> m_cbs;
    };

    class Config{
        public:
            typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

            template<class T>
            static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                    const T& default_value, const std::string& description = "")
            {
                auto it = GetDatas().find(name);
                if(it != GetDatas().end()){
                    // 已经存在的类型和转换的类型不符合，就会返回nullptr；
                    auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                    if(tmp){
                        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "Lookup name=" << name << " exists";
                        return tmp;
                    }else{
                        MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                                << typeid(T).name() << " real_type=" << it->second->getTypeName()
                                << " " << it->second->toString();
                        return nullptr;
                    }
                }

                if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789")
                        != std::string::npos)
                {
                    MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "Lookup name invalid " << name;
                    throw std::invalid_argument(name);
                }

                typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
                GetDatas()[name] = v;
                return v;
            }
            
            template<class T>
            static typename ConfigVar<T>::ptr Lookup(const std::string& name){
                auto it = GetDatas().find(name);

                if(it == GetDatas().end()){
                    return nullptr;
                }

                return std::dynamic_pointer_cast<ConfigVar<T> > (it->second);
            }

            static void LoadFromYaml(const YAML::Node& root);

            static ConfigVarBase::ptr LookupBase(const std::string& name);

        private:
            //  这里为了保证GetDatas()一定被初始化，使用静态函数来调用 -- 5/28；
            static ConfigVarMap& GetDatas(){
                static ConfigVarMap s_datas;
                return s_datas;
            }
            
    };

   

}

#endif