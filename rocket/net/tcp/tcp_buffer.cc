#include <string.h>
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/common/log.h"

namespace rocket {

TcpBuffer::TcpBuffer(int size) : m_size(size) {
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

int TcpBuffer::readAble() {
    return m_write_index - m_read_index;
}
int TcpBuffer::writeAble() {
    return m_size - m_write_index;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
    // buffer不够写了，需要整理扩容
    if (size > writeAble()) {
        int new_size = (size + m_write_index) * 3 / 2;
        resizeBuffer(new_size);
    }

    memcpy(&m_buffer[m_write_index], buf, size);
    m_write_index += size;
}

void TcpBuffer::readFromBuffer(std::vector<char> &re, int size) {
    if (readAble() == 0) {
        return;
    }

    int read_size = std::min(size, readAble());
    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
    re.swap(tmp);

    m_read_index += size;
}

int TcpBuffer::readIndex() {
    return m_read_index;
}

int TcpBuffer::writeIndex() {
    return m_write_index;
}

// 对数组扩容，将可读的内容复制到新数组中
void TcpBuffer::resizeBuffer(int new_size) {
    std::vector<char> tmp(new_size);
    int count = std::min(new_size,readAble());
    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);

    m_size = m_buffer.size();
    m_read_index = 0;
    m_write_index = m_read_index + count;
}

// 当前面以及读过的字节数很多时，进行可读内容的一个平移
void TcpBuffer::adjustBuffer() {
    // 当已经读过的字节占据整个buffer的1/3时进行调整
    if (m_read_index < m_size / 3) {
        return;
    }
    resizeBuffer(m_size);
}

void TcpBuffer::moveReadIndex(int size) {
    int end = m_read_index + size;
    if (end > m_size) {
        ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_size);
        return;
    }

    m_read_index = end;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
    int end = m_write_index + size;
    if (end > m_size) {
        ERRORLOG("moveWriteIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_size);
        return;
    }

    m_write_index = end;
}

}