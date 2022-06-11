//
// Created by liucxi on 2022/6/10.
//

#ifndef LUWU_HTTP_CONNECTION_H
#define LUWU_HTTP_CONNECTION_H

#include "../streams/socket_stream.h"
#include "http.h"

namespace liucxi {
    namespace http {
        class HttpConnection : public SocketStream {
        public:
            typedef std::shared_ptr<HttpConnection> ptr;

            HttpConnection(Socket::ptr socket, bool owner = true);

            HttpResponse::ptr recvResponse();

            size_t sendRequest(const HttpRequest::ptr& req);
        };
    }
}
#endif //LUWU_HTTP_CONNECTION_H
