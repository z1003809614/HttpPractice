#ifndef __MYHTTP_MACRO_H__
#define __MYHTTP_MACRO_H__

#include <string>
#include <assert.h>
#include "util.h"
#include "log.h"

// 双感叹号目的是将其转换为bool类型，就是取两次非的情况
// __builtin_expect(var, target) 表示var是target的概率很大，能够帮助编译器进行代码优化；
#if defined __GNUC__ || defined __llvm__
#   define MYHTTP_LICKLY(x)     __builtin_expect(!!(x), 1)
#   define MYHTTP_UNLICKLY(x)   __builtin_expect(!!(x), 0)
#else
#   define MYHTTP_LICKLY(x)     (x)
#   define MYHTTP_UNLICKLY(x)   (x)
#endif

#define MYHTTP_ASSERT(x) \
    if(MYHTTP_UNLICKLY(!(x))) { \
        MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << myhttp::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define MYHTTP_ASSERT2(x, w) \
    if(MYHTTP_UNLICKLY(!(x))) { \
        MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << myhttp::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif