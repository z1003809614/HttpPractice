#ifndef __MYHTTP_NONCOPYABLE_H__
#define __MYHTTP_NONCOPYABLE_H__

namespace myhttp
{
    class Noncopyable{
        public:
            Noncopyable() = default;
            ~Noncopyable() = default;
            Noncopyable(const Noncopyable&) = delete;
            Noncopyable& operator=(const Noncopyable&) = delete;
    };
} // namespace myhttp


#endif