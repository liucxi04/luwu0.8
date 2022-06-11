//
// Created by liucxi on 2022/6/10.
//
#include "http_connection.h"
#include "http_parser.h"
#include <utility>

namespace liucxi {
    namespace http {
        HttpConnection::HttpConnection(Socket::ptr socket, bool owner)
            : SocketStream(std::move(socket), owner){
        }

        HttpResponse::ptr HttpConnection::recvResponse() {
            HttpResponseParser::ptr parser(new HttpResponseParser);
            uint64_t bufferSize = HttpResponseParser::GetHttpResponseBufferSize();
            std::shared_ptr<char> buffer(new char[bufferSize + 1], [](const char *ptr){ delete[] ptr; });
            char *data = buffer.get();
            size_t offset = 0;
            do {
                size_t len = read(data + offset, bufferSize - offset);
                if (len <= 0) {
                    close();
                    return nullptr;
                }

                len += offset;
                data[len] = '\0';
                size_t nparse = parser->execute(data, len);
                if (parser->getError()) {
                    close();
                    return nullptr;
                }

                offset = len - nparse;
                if (offset == bufferSize) {
                    close();
                    return nullptr;
                }

                if (parser->isFinished()) {
                    break;
                }
            } while (true);
            return parser->getData();
        }

        size_t HttpConnection::sendRequest(const HttpRequest::ptr &req) {
            std::stringstream ss;
            ss << *req;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    }
}
