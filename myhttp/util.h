#ifndef __MYHTTP_UTIL_H
#define __MYHTTP_UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
namespace myhttp
{
    pid_t GetThreadId();
    pid_t GetFiberID();

} // namespace myhttp

#endif