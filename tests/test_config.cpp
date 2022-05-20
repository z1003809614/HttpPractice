#include "../myhttp/config.h"
#include "../myhttp/log.h"

// 全局变量 g_int_value_config;
myhttp::ConfigVar<int>::ptr g_int_value_config = 
    myhttp::Config::Lookup("system.port", (int)8080, "system port");


int main(int argc, char** argv){

    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << g_int_value_config->getValue();
    MYHTTP_LOG_INFO(MYHTTP_LOG_ROOT()) << g_int_value_config->toString();
     
    return 0;
}