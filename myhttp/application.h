/**
 * @file application.h
 * @author Xinjie Nie (www.xinjieer@qq.com)
 * @brief 应用启动操作
 * @version 0.1
 * @date 2022-07-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __MYHTTP_APPLICATION_H__
#define __MYHTTP_APPLICATION_H__

#include "http/http_server.h"

namespace myhttp
{   
    /**
     * @brief 保存应用启动参数，根据参数启动服务
     */
    class Application
    {
        public:

            /**
             * @brief 构造函数
             */
            Application();

            /**
             * @brief  获取当前应用实例
             */
            static Application* GetInstance() { return s_instance; }
            
            /**
             * @brief 对启动参数进行分析处理
             * 
             * @param argc 启动参数个数
             * @param argv 启动参数
             * @return true 参数解析成功
             * @return false 参数解析失败 
             */
            bool init(int argc, char** argv);

            /**
             * @brief 目前封装 daemon，内部调用main函数;
             * 
             * @return true 启动成功
             * @return false 启动失败
             */
            bool run();
        
        private:
            /**
             * @brief 封装了IOManager,调用run_fiber;
             * 
             * @param argc 
             * @param argv 
             * @return int 
             */
            int main(int argc, char** argv);

            /**
             * @brief 根据配置参数，启动http服务器
             * 
             * @return int 
             */
            int run_fiber();

        private:
            /// 启动参数个数
            int m_argc = 0;

            /// 启动参数
            char** m_argv = nullptr;

            /// 服务器实例
            std::vector<myhttp::http::HttpServer::ptr> m_httpservers;

            /// 保存唯一应用实例；
            static Application* s_instance;
    };
} // namespace myhttp



#endif