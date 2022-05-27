#include "../myhttp/config.h"

// 全局变量 g_int_value_config;
myhttp::ConfigVar<int>::ptr g_int_value_config = 
    myhttp::Config::Lookup("system.port", (int)8080, "system port");

myhttp::ConfigVar<float>::ptr g_int_valuex_config = 
    myhttp::Config::Lookup("system.port", (float)8080, "system port");

myhttp::ConfigVar<float>::ptr g_float_value_config = 
    myhttp::Config::Lookup("system.value", (float)10.2f, "system value");

myhttp::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
    myhttp::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

myhttp::ConfigVar<std::list<int> >::ptr g_int_list_value_config = 
    myhttp::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

myhttp::ConfigVar<std::set<int> >::ptr g_int_set_value_config = 
    myhttp::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

myhttp::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config = 
    myhttp::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int unordered_set");

myhttp::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config = 
    myhttp::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

myhttp::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config = 
    myhttp::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int unordered_map");



void print_yaml(const YAML::Node& node,int level){
    if(node.IsScalar()){
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << std::string(level * 4, ' ') 
        << node.Scalar() << " - " << node.Type() << " - " << level << " is Scalar";    
    }else if(node.IsNull()){
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << std::string(level * 4, ' ')
        << "NULL - " << node.Type() << " - " << level << " is Null";
    }else if(node.IsMap()){
        for(auto it = node.begin();it != node.end(); ++it){
             MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << std::string(level * 4, ' ') 
             << it->first << " - " << it->second.Type() << " - " << level << " is Map";
             print_yaml(it->second, level + 1);
        }
    }else if(node.IsSequence()){
        for(size_t i = 0; i< node.size(); ++i){
            MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << std::string(level * 4, ' ')
            << i << " - " << node[i].Type() << " - " << level << " is Sequence";
            print_yaml(node[i] , level + 1);
        } 
    }
}


void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/ubuntu/HttpPractice/bin/conf/log.yml");
    print_yaml(root,0);

    //MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << root;

}


void test_config(){
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v){ \
            MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v){ \
            MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << #prefix " " #name ": {"  \
                    << i.first << " - " << i.second << "}"; \
        } \
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/ubuntu/HttpPractice/bin/conf/log.yml");
    myhttp::Config::LoadFromYaml(root);

    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

class Person{
    public:
        std::string m_name = "";
        int m_age = 0;
        bool m_sex = 0;

        std::string toString() const{
            std::stringstream ss;
            ss << "[Person name=" << m_name
               << " age=" << m_age
               << " sex=" << m_sex
               << "]";
            return ss.str();
        }
};

namespace myhttp{
    template<>
    class LexicalCast<std::string, Person>{
        public:
            Person operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                Person p;
                p.m_name = node["name"].as<std::string>();
                p.m_age = node["age"].as<int>();
                p.m_sex = node["sex"].as<bool>();
                return p;
            }
    };
    template<>
    class LexicalCast<Person, std::string>{
        public:
            std::string operator()(const Person& p){
                YAML::Node node;
                node["name"] = p.m_name;
                node["age"] = p.m_age;
                node["sex"] = p.m_sex;
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };
}

myhttp::ConfigVar<Person>::ptr g_person = 
    myhttp::Config::Lookup("class.person", Person(), "system person");

// 这里错误原因是class.map对应的是sequence，而不是map；
// 1. 要么下面的map改成 ，序列容器；
// 2. 要么yaml文件中，对每个person类都提供一个key；
myhttp::ConfigVar<std::map<std::string, Person> >::ptr g_person_map = 
    myhttp::Config::Lookup("class.map", std::map<std::string, Person>(), "system map person");

// myhttp::ConfigVar<std::vector<Person> >::ptr g_person_vec = 
//     myhttp::Config::Lookup("class.vec", std::vector<Person>(), "system person");

myhttp::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_map_vec = 
    myhttp::Config::Lookup("class.map_vec", std::map<std::string, std::vector<Person> >(), "system map vec person");

void test_class(){
   
// 反斜杠后面有空格会报错；
#define XX_PM(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for(auto& i : m){ \
            MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }

#define XX_PV(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for(auto& i : m){ \
            MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": " << i.toString(); \
        } \
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }

#define XX_PMV(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for(auto& i : m){ \
           for(auto& p : i.second){ \
               MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": " << i.first << " _ " << p.toString(); \
           } \
        } \
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }

    // MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
    // XX_PM(g_person_map, "class.map before");

    
    XX_PMV(g_person_map_vec, "class.map_vec before");
    
    YAML::Node root = YAML::LoadFile("/home/ubuntu/HttpPractice/bin/conf/log.yml");
    myhttp::Config::LoadFromYaml(root);

    // MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    // XX_PM(g_person_map, "class.map after");
    XX_PMV(g_person_map_vec, "class.map_vec after");
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) <<  g_person_map_vec->toString(); \
}



int main(int argc, char** argv){
    
    std::cout << std::endl << "this test_yaml " << std::endl;
    // test_yaml();
    // test_config();
    test_class();
    return 0;
}