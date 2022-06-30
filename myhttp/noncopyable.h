#ifndef __MYHTTP_NONCOPYABLE_H__
#define __MYHTTP_NONCOPYABLE_H__

/**
 * @brief 用于不适合复制的类来继承，简化其他类的代码
 * 
 */
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