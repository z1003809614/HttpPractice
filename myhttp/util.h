#ifndef __MYHTTP_UTIL_H
#define __MYHTTP_UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <vector>
#include <string>
#include <sys/time.h>

namespace myhttp
{
    pid_t GetThreadId();
    pid_t GetFiberId();

    void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
    std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

    //时间ms
    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();

    std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
} // namespace myhttp

#endif