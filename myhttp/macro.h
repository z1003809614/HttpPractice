#ifndef __MYHTTP_MACRO_H__
#define __MYHTTP_MACRO_H__

#include <string>
#include <assert.h>
#include "util.h"
#include "log.h"

#define MYHTTP_ASSERT(x) \
    if(!(x)) { \
        MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << myhttp::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define MYHTTP_ASSERT2(x, w) \
    if(!(x)) { \
        MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << myhttp::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif