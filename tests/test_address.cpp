#include "../myhttp/address.h"
#include "../myhttp/log.h"

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test(){
    std::vector<myhttp::Address::ptr> addrs;
    bool v = myhttp::Address::Lookup(addrs, "www.baidu.com");
    if(!v){
        MYHTTP_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i){
        MYHTTP_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

void test_interface(){
    std::multimap<std::string, std::pair<myhttp::Address::ptr, uint32_t> > results;

    bool v = myhttp::Address::GetInterfaceAddresses(results);

    if(!v){
        MYHTTP_LOG_ERROR(g_logger) << "GetInterFaceAddresses fail";

        return;
    }

    for(auto& i : results){
        MYHTTP_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4(){
    auto addr = myhttp::IPAddress::Create("www.baidu.com");
    if(addr){
        MYHTTP_LOG_INFO(g_logger) << addr->toString();
    }
    return ;
}

int main(int argc, char** argv){
    // test();
    // test_interface();
    test_ipv4();
    return 0;
}