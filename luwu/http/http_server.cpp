//
// Created by liucxi on 2022/6/11.
//
#include "http_server.h"
#include "macro.h"

namespace liucxi {
    namespace http {
        HttpServer::HttpServer(bool keepalive, liucxi::IOManager *worker)
                : TCPServer(worker), m_keepalive(keepalive) {
            m_dispatch.reset(new ServletDispatch);
        }

        void HttpServer::handleClient(const Socket::ptr &sock) {
            HttpSession::ptr session(new HttpSession(sock));
            do {
                auto req = session->recvRequest();
                if (!req) {
                    LUWU_LOG_ERROR(LUWU_LOG_NAME("system")) << "recv http request failed, errno=" << errno
                                                            << " errstr=" << strerror(errno)
                                                            << "client:" << *sock;
                }
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(),
                                                               req->isClose() || !m_keepalive));
                m_dispatch->handle(req, rsp, session);
//                rsp->setBody("liucxi");
                session->sendResponse(rsp);
            } while (m_keepalive);
            session->close();
        }
    }
}
