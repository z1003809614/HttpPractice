# 文件句柄管理

## 模块设计
1. FdCtx，文件句柄上下文类
2. FDManager, 文件句柄管理类：统筹管理所有fd,使用单例模式，全局只有一个；