#include "../myhttp/myhttp.h"
#include <assert.h>

myhttp::Logger::ptr g_logger = MYHTTP_LOG_ROOT();

void test_assert(){
    MYHTTP_LOG_INFO(g_logger) << std::endl <<myhttp::BacktraceToString(10, 0, "      ");
    MYHTTP_ASSERT(false);
}

int main(int argc, char** argv){
    test_assert();
    return 0;
}




