#include "util.h"

namespace myhttp{
    pid_t GetFiberID(){
        return 0;
    }

    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
}