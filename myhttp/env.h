#ifndef __MYHTTP_ENV_H__
#define __MYHTTP_ENV_H__

#include "singleton.h"
#include "thread.h"
#include <map>
#include <vector>

namespace myhttp
{
    class Env{
        public:
            typedef RWMutex RWMutexType;

            /**
             * @brief 将启动参数以key-value的形式保存，并设置m_exe和m_cwd;
             * m_exe的获得使用readlink()库函数来取得系统的进程文件的内容，进而获得相关信息；
             * @param argc 启动参数个数
             * @param argv 启动参数
             */
            bool init(int argc, char** argv);

            /**
             * @brief 添加启动参数
             */
            void add(const std::string& key, const std::string& val);
            
            /**
             * @brief 是否存在key这个参数
             */
            bool has(const std::string& key);

            /**
             * @brief 删除key这个参数
             */
            void del(const std::string& key);

            /**
             * @brief 获得key这个参数，如果不存在则返回default_value;
             */
            std::string get(const std::string& key, const std::string& default_value = "");

            /**
             * @brief 添加key对应的解释说明
             */
            void addHelp(const std::string& key, const std::string& desc);
            
            /**
             * @brief 删除key对应的解释说明
             */
            void removeHelp(const std::string& key);
            
            /**
             * @brief 打印help
             */
            void printHelp();

            /**
             * @brief 获取程序绝对路径
             */
            const std::string getExe() const {return m_exe;}
            
            /**
             * @brief 获取程序绝对路径的父级目录
             */
            const std::string getCwd() const { return m_cwd;}

            /**
             * @brief 设置当前程序的环境变量，内部调用setenv库函数
             */
            bool setEnv(const std::string& key, const std::string& val);

            /**
             * @brief 获取当前程序的环境变量，内部调用getenv库函数
             */
            std::string getEnv(const std::string& key, const std::string& default_value = "");

            /**
             * @brief 获得绝对路径
             */ 
            std::string getAbsolutePath(const std::string& path);
        private:
            /// 读写互斥量
            RWMutexType m_mutex;
            
            /// 启动参数信息
            std::map<std::string, std::string> m_args;
            
            /// 启动参数的解释说明
            std::vector<std::pair<std::string, std::string> > m_helps;

            /// 进程名
            std::string m_program;

            /// 当前进程的启动绝对路径
            std::string m_exe;
            
            /// 当前进程的绝对路径父级目录路径
            std::string m_cwd;
    };

    /// Env类在整个进程中只需要一个实例，故设置为单例模式
    typedef myhttp::Singleton<Env> EnvMgr;
} // namespace myhttp



#endif