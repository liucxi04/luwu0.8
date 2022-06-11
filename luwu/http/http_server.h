//
// Created by liucxi on 2022/6/11.
//

#ifndef LUWU_HTTP_SERVER_H
#define LUWU_HTTP_SERVER_H

#include <utility>

#include "http_session.h"
#include "../tcp_server.h"
#include "servlet.h"

namespace liucxi {
    namespace http {
        class HttpServer : public TCPServer {
        public:
            typedef std::shared_ptr<HttpServer> ptr;

            explicit HttpServer(bool keepalive = false, liucxi::IOManager *worker = liucxi::IOManager::GetThis());

            ServletDispatch::ptr getDispatch() const { return m_dispatch; }

            void setDispatch(ServletDispatch::ptr dispatch) { m_dispatch = std::move(dispatch); }

        protected:
            void handleClient(const Socket::ptr &sock) override;

        private:
            bool m_keepalive;
            ServletDispatch::ptr m_dispatch;
        };
    }
}



#endif //LUWU_HTTP_SERVER_H
