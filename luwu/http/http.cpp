//
// Created by liucx on 2022/6/6.
//

namespace liucxi {
    namespace http {
        HttpMethod StringToHttpMethod(const std::string &m) {

        }

        HttpMethod CharsToHttpMethod(const char *m) {

        }

        std::string HttpMethodToString(const HttpMethod &m) {

        }

        std::string HttpStatusToString(const HttpStatus &s) {

        }

        bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }

    }
}
