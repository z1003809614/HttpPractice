#ifndef __MYHTTP_UTIL_H
#define __MYHTTP_UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <vector>
#include <string>

namespace myhttp
{
    pid_t GetThreadId();
    pid_t GetFiberId();

    void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
    std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

} // namespace myhttp

#endif