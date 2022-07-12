# Log日志模块执行基本原理

## 各模块基本运行原理
1. **LogEvent：** 提供基本的context信息，如线程号，进程号等，并指向一个已经存在的logger;
2. **LogFormater：** 日志打印格式
   1. 构造的时候就指向init()函数，将parttern解析为用vector存储的一系列的items;
   2. format函数会遍历vector<items>,并调用item->format()函数,返回 一个字符串；
   3. item->format()函数，基本上使用的是LogEvent提供的相关方法，获取context信息； 
3. **LogAppender(抽象类)：** 日志输出目标位置
   1. 存有属于自己的level(日志等级)和LogFormater;
   2. 核心函数log()，用于实现对目标位置的日志输出，其内部调用LogFormater->format()；
   3. 存有两个子类，StdoutAppender(控制台输出)和FileLogAppender(文件输出);
4. **Logger类：** 日志器
   1. 成员变量：m_name、m_level、m_appenders(list)、m_formatter、m_root(Logger::ptr);
   2. appender<->Logger是多对多的关系，Logger调用Appender的log()进行输出；
   3. 其主要作用就是打印日志的时候，提供自己的日志名称；
   4. m_root,当日志器自己本身就是root的时候为nullptr,其他非root的logger则指向root;
5. **LoggerManager:** 全局唯一，用于管理Logger；
   1. 初始化的时候会生成root的logger;
   2. 使用map容器保存不同名称的logger;
6. **LogEventWrap类：** 
   1. 避免显示的调用log函数，在其析构函数中调用了log函数；
   2. getSS()函数，其实返回的是LogEvent的stringstream，自定义内容存储到LogEvent::m_ss中,通过LogEvent.getContent()进行读取；

## 日志打印简要流程
                                            LogEventWrap析构  
                                                    |
                                                    v
                                            调用Logger.log() 
                                                    |
                                                    v  
                                    (实现日志打印)调用LogAppender.log()
                                                    |
                                                    v  
                        (返回值：string)调用LogFormater.format(),内部遍历FormaterItem 
                                                    |
                                                    v
                                    FormateItem调用LogEvent相关Api获得信息

## 注意点
1. Logger和Appender独立拥有formatter和level信息，LogEvent拥有独立的level信息；
2. 执行策略：LogEvent::Level >= Logger::Level,有权限向Logger所属Appender进行打印
3. LogEvent::Level >= Logger::Appender::Level,才能够实质性的将日志打印出来；
4. 这里默认Appender的formatter与level与logger一致,其实际操作在Logger::addAppender()函数中；