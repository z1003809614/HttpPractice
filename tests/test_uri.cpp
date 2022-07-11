#include "../myhttp/uri.h"
#include <iostream>

int main(int argc, char** argv){
    myhttp::Uri::ptr uri = myhttp::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar&frg");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << addr->toString() << std::endl;
    return 0;
}