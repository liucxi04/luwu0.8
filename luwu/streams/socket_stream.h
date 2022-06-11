//
// Created by liucxi on 2022/6/10.
//

#ifndef LUWU_SOCKET_STREAM_H
#define LUWU_SOCKET_STREAM_H

#include "stream.h"
#include "socket.h"

namespace liucxi {
    class SocketStream : public Stream {
    public:
        typedef std::shared_ptr<SocketStream> ptr;

        explicit SocketStream(Socket::ptr socket, bool owner = true);

        ~SocketStream() override;

        size_t read(void *buffer, size_t length) override;

        size_t read(ByteArray::ptr ba, size_t length) override;

        size_t write(const void *buffer, size_t length) override;

        size_t write(ByteArray::ptr ba, size_t length) override;

        void close() override;

        Socket::ptr getSocket() const { return m_socket; }

        bool isConnected() const;

    private:
        Socket::ptr m_socket;
        bool m_owner;
    };
}
#endif //LUWU_SOCKET_STREAM_H
