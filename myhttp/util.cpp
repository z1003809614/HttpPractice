#include <execinfo.h>
#include "util.h"
#include "log.h"
#include "fiber.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

namespace myhttp{
    
    static myhttp::Logger::ptr g_logger = MYHTTP_LOG_NAME("system");

    pid_t GetFiberId(){
        return myhttp::Fiber::GetFiberId();
    }

    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip){
        void** array = (void**) malloc((sizeof(void*) * size));
        size_t s = ::backtrace(array, size); 

        char** strings = backtrace_symbols(array, s);
        if(strings == NULL){
            MYHTTP_LOG_ERROR(g_logger) << "backtrace_synbols error";
            free(strings);
            free(array);
            return;
        }

        for(size_t i = skip; i < s; ++i){
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);
    }
    std::string BacktraceToString(int size, int skip, const std::string& prefix){
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for(size_t i = 0; i < bt.size(); ++i){
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }


    uint64_t GetCurrentMS(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
    }
    uint64_t GetCurrentUS(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
    }

    std::string Time2Str(time_t ts, const std::string& format){
        struct tm tm;
        localtime_r(&ts, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }


    void FSUtil::ListAllFile(std::vector<std::string>& files, 
                                    const std::string& path,
                                    const std::string& subfix)
    {
        if(access(path.c_str(), 0) != 0){
            return;
        }
        DIR* dir = opendir(path.c_str());
        if(dir == nullptr){
            return;
        }
        struct dirent* dp = nullptr;
        while((dp = readdir(dir)) != nullptr){
            if(dp->d_type == DT_DIR){
                if(!strcmp(dp->d_name, ".")
                    || !strcmp(dp->d_name, "..")){
                    continue;
                }
                ListAllFile(files, path + "/" + dp->d_name, subfix);
            }else if(dp->d_type == DT_REG){
                std::string filename(dp->d_name);
                if(subfix.empty()){
                    files.push_back(path + "/" + filename);
                }else{
                    if(filename.size() < subfix.size()){
                        continue;
                    }
                    if(filename.substr(filename.length() - subfix.size()) == subfix){
                        files.push_back(path + "/" + filename);
                    }
                }
            }
        }
        closedir(dir);
    }

    static int __lstat(const char* file, struct stat* st = nullptr) {
        struct stat lst;
        // lstat,返回连接文件的相关信息；0 成功，-1失败(表示该文件不是链接文件)
        int ret = lstat(file, &lst);
        if(st) {
            *st = lst;
        }
        return ret;
    }

    static int __mkdir(const char* dirname) {
        if(access(dirname, F_OK) == 0) {
            return 0;
        }
        return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    bool FSUtil::IsRunningPidfile(const std::string& pidfile){
        if(__lstat(pidfile.c_str()) != 0) {
            return false;
        }
        
        std::ifstream ifs(pidfile);
        std::string line;
        
        //文件没打开，或者读取失败
        if(!ifs || !std::getline(ifs, line)) {
            return false;
        }
        // 没读到数据
        if(line.empty()) {
            return false;
        }
        // 转换为pid信息；
        pid_t pid = atoi(line.c_str());
        if(pid <= 1) {
            return false;
        }

        // kill执行失败；信号0,没有明确定义，类似与ping，是检测进程是否存在的一种手段；
        if(kill(pid, 0) != 0) {
            return false;
        }

        return true;
    }

    bool FSUtil::Mkdir(const std::string& dirname) {
        // 该目录已经构建好了
        if(__lstat(dirname.c_str()) == 0) {
            return true;
        }
        
        // strdup()会先用maolloc()配置与参数s 字符串相同的空间大小，
        // 然后将参数s 字符串的内容复制到该内存地址，然后把该地址返回。
        // 该地址最后可以利用free()来释放。
        char* path = strdup(dirname.c_str());
        
        // C 库函数 char *strchr(const char *str, int c) 
        // 在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置
        char* ptr = strchr(path + 1, '/');
        do {
            // 层层构建工作目录；
            for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
                *ptr = '\0';
                if(__mkdir(path) != 0) {
                    break;
                }
            }
            if(ptr != nullptr) {
                break;
            } else if(__mkdir(path) != 0) {
                break;
            }
            free(path);
            return true;
        
        } while(0);
        
        free(path);
        return false;
    }
}