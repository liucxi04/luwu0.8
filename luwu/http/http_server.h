//
// Created by liucxi on 2022/6/11.
//

#ifndef LUWU_HTTP_SERVER_H
#define LUWU_HTTP_SERVER_H

#include "http_session.h"
#include "../tcp_server.h"

namespace liucxi {
    namespace http {
        class HttpServer : public TCPServer {
        public:
            typedef std::shared_ptr<HttpServer> ptr;

            HttpServer(bool keepalive = false, liucxi::IOManager *worker = liucxi::IOManager::GetThis());

        protected:
            void handleClient(const Socket::ptr &sock) override;

        private:
            bool m_keepalive;
        };
    }
}



#endif //LUWU_HTTP_SERVER_H
