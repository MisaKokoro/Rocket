#ifndef ROCKET_NET_TCP_TCP_BUFFER_H
#define ROCKET_NET_TCP_TCP_BUFFER_H

#include <vector>
namespace rocket {

class TcpBuffer {
public:
    TcpBuffer(int size);
    ~TcpBuffer();

    int readAble();
    int writeAble();

    void wirteToBuffer(const char* buf, int size);
    void readFromBuffer(std::vector<char> &re, int size);

    int readIndex();
    int writeIndex();

    void resizeBuffer(int new_size);
    void adjustBuffer();

    void moveReadIndex(int size);
    void moveWriteIndex(int size);
private:
    int m_size {0};
    int m_read_index {0};
    int m_write_index {0};

    std::vector<char> m_buffer;
};
}
#endif