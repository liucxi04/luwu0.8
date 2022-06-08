//
// Created by liucxi on 2022/6/8.
//

#ifndef LUWU_TCP_SERVER_H
#define LUWU_TCP_SERVER_H

#include <memory>
#include <utility>
#include "iomanager.h"
#include "address.h"
#include "socket.h"

namespace liucxi {

    class TCPServer : public std::enable_shared_from_this<TCPServer>, Noncopyable {
    public:
        typedef std::shared_ptr<TCPServer> ptr;

        explicit TCPServer(IOManager *worker = IOManager::GetThis());

        virtual ~TCPServer();

        virtual bool bind(const Address::ptr &address);

        virtual bool bind(const std::vector<Address::ptr> &addresses);

        virtual bool start();

        virtual void stop();

        uint64_t getRecvTimeout() const { return m_recvTimeout; }

        void setRecvTimeout(uint64_t timeout) { m_recvTimeout = timeout; }

        std::string getName() const { return m_name; }

        void setName(std::string name) { m_name = std::move(name); }

        bool isStop() const { return m_stop; }

        virtual std::string toString();

    protected:
        virtual void handleClient(const Socket::ptr &client);

        virtual void startAccept(const Socket::ptr &sock);

    protected:
        std::vector<Socket::ptr> m_socks;
        IOManager *m_worker;
        uint64_t m_recvTimeout;
        std::string m_name;
        bool m_stop;
    };
}
#endif //LUWU_TCP_SERVER_H
