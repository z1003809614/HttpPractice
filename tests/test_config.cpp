#include "../myhttp/config.h"

// 全局变量 g_int_value_config;
myhttp::ConfigVar<int>::ptr g_int_value_config = 
    myhttp::Config::Lookup("system.port", (int)8080, "system port");

myhttp::ConfigVar<float>::ptr g_float_value_config = 
    myhttp::Config::Lookup("system.value", (float)10.2f, "system value");

myhttp::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
    myhttp::Config::Lookup("system.int_vec", std::vector<int>(1,2), "system int vec");



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
    auto v = g_int_vec_value_config->getValue();
    for(auto& i : v){
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "before int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("/home/ubuntu/HttpPractice/bin/conf/log.yml");
    myhttp::Config::LoadFromYaml(root);

    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    v = g_int_vec_value_config->getValue();
    for(auto& i : v){
        MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << "after int_vec: " << i;
    }
}


int main(int argc, char** argv){
    
    std::cout << std::endl << "this test_yaml " << std::endl;
    // test_yaml();
    test_config();

    return 0;
}