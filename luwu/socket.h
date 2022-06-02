//
// Created by liucxi on 2022/6/2.
//

#ifndef LUWU_SOCKET_H
#define LUWU_SOCKET_H

#include "noncopyable.h"
#include "address.h"
#include <memory>
#include <vector>

namespace liucxi {
    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
    public:
        typedef std::shared_ptr<Socket> ptr;
        typedef std::weak_ptr<Socket> weak_ptr;

        static Socket::ptr CreateTCP();

        static Socket::ptr CreateUDP();

        static Socket::ptr CreateTCP6();

        static Socket::ptr CreateUDP6();

        static Socket::ptr CreateTCP(const Address::ptr& addr);

        static Socket::ptr CreateUDP(const Address::ptr& addr);

        static Socket::ptr CreateUnixTCP();

        static Socket::ptr CreateUnixUDP();

        Socket(int family, int type, int protocol = 0);

        ~Socket();

        uint64_t getSendTimeout() const;

        void setSendTimeout(uint64_t timeout);

        uint64_t getRecvTimeout() const;

        void setRecvTimeout(uint64_t timeout);

        bool getOption(int level, int option, void *result, size_t *len) const;

        template<typename T>
        bool getOption(int level, int option, T &value) {
            ssize_t len = sizeof(T);
            return getOption(level, option, &value, &len);
        }

        bool setOption(int level, int option, const void *result, size_t len) const;

        template<typename T>
        bool setOption(int level, int option, const T &value) {
            return setOption(level, option, &value, sizeof(T));
        }

        Socket::ptr accept() const;

        bool bind(const Address::ptr& addr);

        bool connect(const Address::ptr& addr, int64_t timeout = -1);

        bool close();

        bool listen(int backlog = SOMAXCONN) const;

        ssize_t send(const void *buffer, size_t length, int flags = 0) const;

        ssize_t sendTo(const void *buffer, size_t length, const Address::ptr& to, int flags = 0) const;

        ssize_t recv(void *buffer, size_t length, int flags = 0) const;

        ssize_t recvFrom(void *buffer, size_t length, const Address::ptr& from, int flags = 0) const;

        Address::ptr getRemoteAddress();

        Address::ptr getLocalAddress();

        int getSocket() const { return m_sock; }

        int getFamily() const { return m_family; }

        int getType() const { return m_type; }

        int getProtocol() const { return m_protocol; }

        bool isConnected() const { return m_isConnected; }

        bool isValid() const;

        int getError() const;

        std::ostream &dump(std::ostream &os) const;

        bool cancelRead() const;

        bool cancelWrite() const;

        bool cancelAccept() const;

        bool cancelAll() const;

    private:

        void initSock();

        bool init(int sock);

        void newSock();

    private:
        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConnected;

        Address::ptr m_localAddress;
        Address::ptr m_remoteAddress;
    };
}
#endif //LUWU_SOCKET_H
