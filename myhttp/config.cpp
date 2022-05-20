#include "config.h"

namespace myhttp{

    // 由于是静态资源，在test_config中，调用静态函数，该函数内部操作 静态成员变量，静态成员变量必须要显式的声明，才会初始化，否则无法进行操作；故这里需要声明；
    Config::ConfigVarMap Config::s_datas;

}