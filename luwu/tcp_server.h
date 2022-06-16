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

    /**
     * @brief TCP 服务器封装
     */
    class TCPServer : public std::enable_shared_from_this<TCPServer>, Noncopyable {
    public:
        typedef std::shared_ptr<TCPServer> ptr;

        explicit TCPServer(IOManager *worker = IOManager::GetThis(),
                           IOManager *accept = IOManager::GetThis());

        virtual ~TCPServer();

        virtual bool bind(Address::ptr address);

        virtual bool bind(const std::vector<Address::ptr> &addresses);

        virtual bool start();

        virtual void stop();

        uint64_t getRecvTimeout() const { return m_recvTimeout; }

        void setRecvTimeout(uint64_t timeout) { m_recvTimeout = timeout; }

        std::string getName() const { return m_name; }

        virtual void setName(std::string name) { m_name = std::move(name); }

        bool isStop() const { return m_stop; }

        virtual std::string toString();

    protected:
        virtual void handleClient(Socket::ptr client);

        virtual void startAccept(Socket::ptr sock);

    protected:
        std::vector<Socket::ptr> m_socks;   /// 监听 socket 数组
        IOManager *m_worker;                /// 新连接的 socket 的工作调度器
        IOManager *m_accept;                /// 接受 socket 连接的调度器
        uint64_t m_recvTimeout;             /// 接收超时时间
        std::string m_name;                 /// 服务器名称
        bool m_stop;                        /// 服务器是否停止
    };
}
#endif //LUWU_TCP_SERVER_H
