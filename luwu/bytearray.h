//
// Created by liucxi on 2022/6/3.
//

#ifndef LUWU_BYTEARRAY_H
#define LUWU_BYTEARRAY_H

#include <memory>
#include <string>
#include <vector>
#include <sys/socket.h>

namespace liucxi {
    class ByteArray {
    public:
        typedef std::shared_ptr<ByteArray> ptr;

        struct Node {
            Node();

            explicit Node(size_t s);

            ~Node();

            char *ptr;
            Node *next;
            size_t size;
        };

        explicit ByteArray(size_t base_size = 4096);

        ~ByteArray();

        void writeFixInt8(int8_t val);

        void writeFixUint8(uint8_t val);

        void writeFixInt16(int16_t val);

        void writeFixUint16(uint16_t val);

        void writeFixInt32(int32_t val);

        void writeFixUint32(uint32_t val);

        void writeFixInt64(int64_t val);

        void writeFixUint64(uint64_t val);

        void writeInt32(int32_t val);

        void writeUint32(uint32_t val);

        void writeInt64(int64_t val);

        void writeUint64(uint64_t val);

        void writeFloat(float val);

        void writeDouble(double val);

        void writeStringFix16(const std::string &val);

        void writeStringFix32(const std::string &val);

        void writeStringFix64(const std::string &val);

        void writeStringVint(const std::string &val);

        void writeStringWithoutLength(const std::string &val);

        int8_t readFixInt8();

        uint8_t readFixUint8();

        int16_t readFixInt16();

        uint16_t readFixUint16();

        int32_t readFixInt32();

        uint32_t readFixUint32();

        int64_t readFixInt64();

        uint64_t readFixUint64();

        int32_t readInt32();

        uint32_t readUint32();

        int64_t readInt64();

        uint64_t readUint64();

        float readFloat();

        double readDouble();

        std::string readStringFix16();

        std::string readStringFix32();

        std::string readStringFix64();

        std::string readStringVint();

        void clear();

        void write(const void *buf, size_t size);

        void read(void *buf, size_t size);

        void read(void *buf, size_t size, size_t position) const;

        bool writeToFile(const std::string &name) const;

        bool readFromFile(const std::string &name);

        size_t getBaseSize() const { return m_baseSize; }

        size_t getReadSize() const { return m_size - m_position; }

        size_t getPosition() const { return m_position; }

        void setPosition(size_t pos);

        bool isLittleEndian() const { return m_endian == LITTLE_ENDIAN; }

        void setLittleEndian(bool v);

        size_t getSize() const { return m_size; }

        std::string toString() const;

        uint64_t getReadBuffers(std::vector<iovec> &buffers, uint64_t len = ~0ull);

        uint64_t getReadBuffers(std::vector<iovec> &buffers, uint64_t len, uint64_t position);

        uint64_t getWriteBuffers(std::vector<iovec> &buffers, uint64_t len);

    private:
        void addCapacity(size_t size);

        size_t getCapacity() const { return m_capacity; }

    private:
        size_t m_baseSize;
        size_t m_position;
        size_t m_capacity;
        size_t m_size;
        uint32_t m_endian;
        Node *m_root;
        Node *m_cur;
    };
}

#endif //LUWU_BYTEARRAY_H
