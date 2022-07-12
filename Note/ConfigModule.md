# Config模块设计原理

## 1. 基本模块说明
1. **ConfigVarBase:** 单个配置项的基类(这个基类的抽象感觉意义不大)
   1. 保存单个配置项的参数名称m_name和描述m_description
2. **ConfigVar:** 继承ConfigVarBase，含有模板参数<T, FromStr, ToStr>，具化的单个配置项
   1. 保存T类型数据，m_cbs使用map容器存储的一系列回调函数
   2. m_cbs主要用于当该配置项发生变化时，应该执行的某些操作；
3. **Config:** :提供便捷的方法创建/访问Configvar
   1. 保存ConfigVarMap类型的配置项数据
   2. 该类主要提供**静态方法Lookup**进行使用，此时会涉及到Config::m_data,Config::m_cbs,由于类内变量的初始化需要显示构造的时候才进行，为了避免未定义行为的出现，这里使用**静态方法+方法内部静态变量**的行为来实现提前初始化；
4. **LexicalCast:** Yaml String与class T的互相转化
   1. 基础转换使用boost::lexical_cast<T>实现；
   2. 如果需要实现自定义类型的互相转换，则需要自定义相关LexicalCast偏特化模板；
   3. 其内部主要使用了std::stringstream来充当中间转换的介质，并使用了Yaml::Node的相关特性；
5. **LoadFromYaml:** 根据Yaml::Node的值，初始化内部定义的ConfigVar
   1. 内部使用ListAllMember(),该函数递归的将Yaml::Node进行遍历并存储；
   2. 上面的方法会将所有层级的node都进行层层分解保留,即有很多的数据会重复保存，但是，我们进行更新的时候，只会对某一个层级进行更新；
   3. 上面的例子如: 
```
        层级结构如下：
            key 1:
                key 2:
                    value
        会保存为: 
            node1 = {key1, YAML::NODE({key2, value})} 
            node2 = {key1.key2, YAML::NODE(value)}
```    

## 2. 运行的流程
```
                使用Config::Lookup()注册配置项，添加Listener回调函数
                                    |
                                    v
                   调用YAML::LoadFile(configFilePath)获取配置信息
                                    |
                                    V
         调用Config::LoadFromYaml(Yaml::Node)对注册了的配置项进行调整
                                    |
                                    V
  调用ConfigVar::setValue()(该函数在fromString中调用)，则会执行ConfigVar::m_cbs回调函数
```

## 3. 相关知识点
1. typeid(expresion)
   1. 用于获取表达式的类型信息，return struct type_info类型，包含一些数据信息
2. std::transform(first1, last1, result, op_increase)
   1. 指定的范围内应用于给定的操作，并将结果存储在指定的另一个范围内
 
