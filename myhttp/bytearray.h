#ifndef __MYHTTP_BYTEARRAY_H__
#define __MYHTTP_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/uio.h>
#include <vector>

namespace myhttp
{   /**
    * @brief 序列化与反序列化的类，
    *   主要实现了读写int float double, string等类型的变量； 
    *   使得后续解析http的内容的时候，能够快速的调用其中的方法，达到内容的快速解析；
    * 设计思路：使用节点链表的形式进行字符内容存储；
    */
    class ByteArray
    {
    public:
        typedef std::shared_ptr<ByteArray> ptr;

        // 存储内容的节点；
        struct Node{
            Node(size_t s);
            Node();
            ~Node();

            char* ptr;
            Node* next;
            size_t size;
        };
        
        ByteArray(size_t base_size = 4096);
        ~ByteArray();

        // write
        // Fix表示固定，即固定位数；
        void writeFint8(int8_t value);
        void writeFuint8(uint8_t value);
        void writeFint16(int16_t value);
        void writeFuint16(uint16_t value);
        void writeFint32(int32_t value);
        void writeFuint32(uint32_t value);
        void writeFint64(int64_t value);
        void writeFuint64(uint64_t value);
        // 这里的方法用于不定位数的int类；(使用了7bit压缩的int类型(可占用1~5字节))
        void writeInt32( int32_t value);
        void writeUint32( uint32_t value);
        void writeInt64( int64_t value);
        void writeUint64( uint64_t value);

        void writeFloat( float value);
        void writeDouble( double value);
        void writeString16(const std::string& value);
        void writeString32(const std::string& value);
        void writeString64(const std::string& value);
        void writeStringVint(const std::string& value);
        void writeStringWithoutLength(const std::string& value);

        // read
        int8_t readFint8();
        uint8_t readFuint8();
        int16_t readFint16();
        uint16_t readFuint16();
        int32_t readFint32();
        uint32_t readFuint32();
        int64_t readFint64();
        uint64_t readFuint64();

        int32_t readInt32();
        uint32_t readUint32();
        int64_t readInt64();
        uint64_t readUint64();

        float readFloat();
        double readDouble();

        std::string readStringF16();
        std::string readStringF32();
        std::string readStringF64();
        std::string readStringVint();

        // 内部操作
        // 重置当前类；
        void clear();

        /**
         * @brief 从buf中向节点写入数据
         * @param buf 数据缓存池；
         * @param size buf的大小；
         */
        void write(const void* buf, size_t size);
        void read(void* buf, size_t size);
        
        /**
         * @brief 从position位置读取size个字节数据到buf中；
         * @param buf 
         * @param size 
         * @param position 
         */
        void read(void* buf, size_t size, size_t position) const;

        size_t getPosition() const { return m_position; }
        void setPosition(size_t v);

        bool writeToFile(const std::string& name) const;
        bool readFromFile(const std::string& name);

        // 返回Node可存储的数据大小；
        size_t getBaseSize() const { return m_baseSize;}
        // 返回可读的数据长度；
        size_t getReadSize() const { return m_size - m_position; }

        bool isLittleEndian() const;
        void setIsLittlEndian(bool val);

        std::string toString() const;
        std::string toHexString() const;

        uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
        uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
        uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

        size_t getSize() const { return m_size; }
    private:
        // 拓展容量直至能够满足size字节的存储；
        void addCapacity(size_t size);
        // 返回可用的容量；
        size_t getCapacity() const {return m_capacity - m_position;}

    private:
        /* data */
        size_t m_baseSize;      // 每一个node存放数据的大小
        size_t m_position;      // 当前操作的数据位置；
        size_t m_capacity;      // 容量大小；应该是baseSize的倍数；
        size_t m_size;          // 真实存放的数据的大小；小于等于capacity;
        int8_t m_endian;        // 大小端模式；

        Node* m_root;           // 链表头节点；
        Node* m_cur;            // 当前操作的节点；
    };
    
} // namespace myhttp


#endif