#include "config.h"

namespace myhttp{

    // 由于是静态资源，在test_config中，调用静态函数，该函数内部操作 静态成员变量，静态成员变量必须要显式的声明，才会初始化，否则无法进行操作；故这里需要声明；
    // 在这里初始化静态资源，如果使用静态库链接，会出现段错误，放到test_config.cpp中进行初始化就没问题；现在将其转移到.h文件中进行初始化 -- 2022/5/24
    // Config::ConfigVarMap Config::s_datas;

    /* ConfigVarBase::ptr Config::LookupBase(const std::string& name){
        auto it = s_datas.find(name);
        return it == s_datas.end() ? nullptr : it->second;
    }

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

    void Config::LoadFromYaml(const YAML::Node& root){
        std::list<std::pair<std::string, const YAML::Node> > all_nodes;
        ListAllMember("", root, all_nodes); 

        for(auto& i : all_nodes){
            std::string key = i.first;
            if(key.empty()){
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
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
    } */

}