cmake_minimum_required(VERSION 2.8)
project(myhttp)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# 头文件搜索路径；
include_directories(.)


 # 设置 变量 LIB_SRC
set(LIB_SRC
    myhttp/log.cpp
    myhttp/util.cpp
    myhttp/config.cpp
    myhttp/thread.cpp
    myhttp/fiber.cpp
    myhttp/scheduler.cpp
    myhttp/iomanager.cpp
    myhttp/timer.cpp
    myhttp/hook.cpp
    myhttp/fd_manager.cpp
    myhttp/address.cpp
    myhttp/socket.cpp
    myhttp/bytearray.cpp
    myhttp/http/http.cpp
    myhttp/http/http_parser.cpp
    myhttp/tcp_server.cpp
    myhttp/stream.cpp
    myhttp/socket_stream.cpp
    myhttp/http/http_session.cpp
    myhttp/http/http_server.cpp
    myhttp/http/servlet.cpp
    myhttp/http/http_connection.cpp
    myhttp/daemon.cpp
    )

ragelmaker(myhttp/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/myhttp/http)
ragelmaker(myhttp/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/myhttp/http)
ragelmaker(myhttp/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/myhttp)


# 源文件.c -> 预处理文件.i -> 汇编文件.s -> 机器码文件.o -> 可执行文件；
# 生成 动态库文件；静态库用 STATIC关键字；
add_library(myhttp SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(myhttp) #__FILE__
target_link_libraries(myhttp yaml-cpp pthread dl)
# 不加这句，会导致 编译 test出错；按道理 test并未使用config相关文件，即没有用到yaml相关函数，怎么会报错呢？ 
# 原因是 test 使用 myhttp库，而myhttp库是包含config的相关信息的； 
# 前期 生成 config.o 文件未经过链接阶段，不会报错；
# 动态库 和 静态库 个人感觉应该是把.o文件中所有信息进行一个整合而已，并且也不进行链接阶段；所有也不会报错；
# 库（无论动态或静态）中的函数如果调用了其他库，那就需要看该库的编译的时候使用的命令，如果是-static即静态链接的，优先使用静态，并将其内容copy一份，否则只是调用关系；
# 所以到了 编译 test 文件的时候，link myhttp的时候，才发现某些符号未定义，而报错；
# 在这里添加link信息，可以保证后续代码无需重复添加 yaml-cpp的依赖信息，因为直观上看 test是不依赖yaml-cpp的，所以不应该将此处的依赖，推迟到下级；

#add_library(myhttp_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (myhttp_static PROPERTIES OUTPUT_NAME "myhttp")

# 生成 可执行文件 test ；
add_executable(test tests/test.cpp)
# 提示编译器 先生成 myhttp 依赖，再生成可执行文件；
add_dependencies(test myhttp)
force_redefine_file_macro_for_sources(test) #__FILE__
# 设置  target需要连接的库；
target_link_libraries(test myhttp)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config myhttp)
force_redefine_file_macro_for_sources(test_config) #__FILE__
target_link_libraries(test_config myhttp yaml-cpp)

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread myhttp)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread myhttp)

add_executable(test_utils tests/test_utils.cpp)
add_dependencies(test_utils myhttp)
force_redefine_file_macro_for_sources(test_utils) #__FILE__
target_link_libraries(test_utils myhttp)

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber myhttp)
force_redefine_file_macro_for_sources(test_fiber) #__FILE__
target_link_libraries(test_fiber myhttp)

add_executable(test_scheduler tests/test_scheduler.cpp)
add_dependencies(test_scheduler myhttp)
force_redefine_file_macro_for_sources(test_scheduler) #__FILE__
target_link_libraries(test_scheduler myhttp)


add_executable(test_iomanager tests/test_iomanager.cpp)
add_dependencies(test_iomanager myhttp)
force_redefine_file_macro_for_sources(test_iomanager) #__FILE__
target_link_libraries(test_iomanager myhttp)

add_executable(test_hook tests/test_hook.cpp)
add_dependencies(test_hook myhttp)
force_redefine_file_macro_for_sources(test_hook) #__FILE__
target_link_libraries(test_hook myhttp)

add_executable(test_address tests/test_address.cpp)
add_dependencies(test_address myhttp)
force_redefine_file_macro_for_sources(test_address) #__FILE__
target_link_libraries(test_address myhttp)

add_executable(test_socket tests/test_socket.cpp)
add_dependencies(test_socket myhttp)
force_redefine_file_macro_for_sources(test_socket) #__FILE__
target_link_libraries(test_socket myhttp)

add_executable(test_bytearray tests/test_bytearray.cpp)
add_dependencies(test_bytearray myhttp)
force_redefine_file_macro_for_sources(test_bytearray) #__FILE__
target_link_libraries(test_bytearray myhttp)

add_executable(test_http tests/test_http.cpp)
add_dependencies(test_http myhttp)
force_redefine_file_macro_for_sources(test_http) #__FILE__
target_link_libraries(test_http myhttp)

add_executable(test_http_parser tests/test_http_parser.cpp)
add_dependencies(test_http_parser myhttp)
force_redefine_file_macro_for_sources(test_http_parser) #__FILE__
target_link_libraries(test_http_parser myhttp)

add_executable(test_tcpserver tests/test_tcpserver.cpp)
add_dependencies(test_tcpserver myhttp)
force_redefine_file_macro_for_sources(test_tcpserver) #__FILE__
target_link_libraries(test_tcpserver myhttp)

add_executable(echo_server examples/echo_server.cpp)
add_dependencies(echo_server myhttp)
force_redefine_file_macro_for_sources(echo_server) #__FILE__
target_link_libraries(echo_server myhttp)


add_executable(test_http_server tests/test_http_server.cpp)
add_dependencies(test_http_server myhttp)
force_redefine_file_macro_for_sources(test_http_server) #__FILE__
target_link_libraries(test_http_server myhttp)

add_executable(test_http_connection tests/test_http_connection.cpp)
add_dependencies(test_http_connection myhttp)
force_redefine_file_macro_for_sources(test_http_connection) #__FILE__
target_link_libraries(test_http_connection myhttp)

add_executable(test_uri tests/test_uri.cpp)
add_dependencies(test_uri myhttp)
force_redefine_file_macro_for_sources(test_uri) #__FILE__
target_link_libraries(test_uri myhttp)

add_executable(test_daemon tests/test_daemon.cpp)
add_dependencies(test_daemon myhttp)
force_redefine_file_macro_for_sources(test_daemon) #__FILE__
target_link_libraries(test_daemon myhttp)
# 设置输出文件的路径；
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
