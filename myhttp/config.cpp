#include "config.h"

namespace myhttp{

    // 由于是静态资源，在test_config中，调用静态函数，该函数内部操作 静态成员变量，静态成员变量必须要显式的声明，才会初始化，否则无法进行操作；故这里需要声明；
    // 在这里初始化静态资源，如果使用静态库链接，会出现段错误，放到test_config.cpp中进行初始化就没问题；现在将其转移到.h文件中进行初始化 -- 2022/5/24
    // 将下面的语句转至 .h 文件中 在使用共享库中，会导致两次对静态变量的析构问题，猜测原因是，congfig.cpp和test_congfig.cpp都引用了config.h，
    // 导致在程序执行完毕后，执行程序就析构了静态变量，然后释放 共享库的时候又 析构了一次该静态变量，即出现上面的问题；
    // Config::ConfigVarMap Config::s_datas();

    // 查询key是否已经存在；
    ConfigVarBase::ptr Config::LookupBase(const std::string& name){
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        return it == GetDatas().end() ? nullptr : it->second;
    }

    // 加载yaml文件，并根据内容，更新s_data中已经存储的值；
    // 只修改已经约定过的数据，所以其 类型 是已知的；只需要将配置文件的字符串，转换为对应的类型即可；
    void Config::LoadFromYaml(const YAML::Node& root){
        std::list<std::pair<std::string, const YAML::Node> > all_nodes;
        ListAllMember("", root, all_nodes); 

        for(auto& i : all_nodes){
            std::string key = i.first;
            //std::cout << "key: " << key << std::endl;

            if(key.empty()){
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            // 如果能找到key，就表示该数据有用；
            ConfigVarBase::ptr var = LookupBase(key);

            if(var){
                if(i.second.IsScalar()){
                    var->fromString(i.second.Scalar());
                }else{
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    void Config::visit(std::function<void(ConfigVarBase::ptr)> cb){
        RWMutexType::ReadLock lock(GetMutex());
        ConfigVarMap& m = GetDatas();
        for(auto it = m.begin(); it != m.end(); ++it){
            cb(it->second);
        }
    }

    // 辅助函数，用于LoadFromYaml

    // 把yaml结点的属性构造成（一级.二级.三级：value）的样式；使用了深度优先的一种遍历思路；并且其存储(key,node)，
    // 其中有些key是中间值，如(sytem,node)这样数据，我们是不会用到的；所以可以进行优化；
    // 筛选有用数据的过程交给了 LoadFromYaml 函数来处理；
    static void ListAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string,const YAML::Node> >& output){
        if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789")
                != std::string::npos){
                    MYHTTP_LOG_ERROR(MYHTTP_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
                    return;
        }
        output.push_back(std::make_pair(prefix, node));
        if(node.IsMap()){
            for(auto it = node.begin(); it != node.end(); ++it){
                ListAllMember(prefix.empty() ? (it->first.Scalar()) : (prefix + "." + (it->first.Scalar())), it->second, output);
            }
        }                                
    }


}