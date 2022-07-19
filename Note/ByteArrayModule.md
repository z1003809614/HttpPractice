# 二进制数组序列化

## 模块设计
1. Node数据节点
   1. char* ptr -> 数据实际存放的char地址
   2. Node* next -> 下一个Node节点地址
   3. size_t size -> char数组的大小
2. 通过以下变量用链表的来模拟一个大型序列化容器；
   1. m_baseSize: 一个数据节点最大的大小
   2. m_position: 当前操作的位置 (感觉是这个意思，当前序列总共存储了多少个字节)；
   3. m_capacity: 当前序列总共开辟了多少个字节的内存；
   4. m_size: 当前数据总共存储了多少个字节；感觉和m_position重复；
   5. m_endian: 使用大端还是小段进行存储；
   6. m_root: 链表头节点
   7. m_cur: 当前操作的节点；
3. 封装基本类型的读写操作，
   1. 普通类型定长数据(int float double string)的读写操作
   2. 使用zigzag算法提供了对变长int的读写操作；
   3. 提供了针对iovec类型的buffer进行读写操作；