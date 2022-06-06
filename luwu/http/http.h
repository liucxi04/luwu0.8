//
// Created by liucx on 2022/6/6.
//

#ifndef LUWU_HTTP_H
#define LUWU_HTTP_H

#include <memory>
#include <string>
#include <map>

namespace liucxi {
    namespace http {
        enum class HttpMethod {
#define XX(num, name, string) name = num,
            HTTP_METHOD_MAP(XX)
#undef
            INVALID_METHOD
        };

        enum class HttpStatus {
#define XX(code, name, desc) name = code,
            HTTP_STATUS_MAP(XX
#undef
        };

        static HttpMethod StringToHttpMethod(const std::string &m);

        static HttpMethod CharsToHttpMethod(const char *m);

        static std::string HttpMethodToString(const HttpMethod &m);

        static std::string HttpStatusToString(const HttpStatus &s);

        struct CaseInsensitiveLess {
            bool operator()(const std::string &lhs, const std::string &rhs) const;
        };

        template<typename T>
        bool checkGetAs(const MapType &m, const std::string &key, T &val, const T &def = T()) {
            auto it = m.find(key);
            if (it == m.end()) {
                val = def;
                return false;
            }
            try {
                val = boost::lexical_cast<T>(it->second);
            } catch (...) {
                val = def;
            }
            return false;
        }

        template<typename T>
        T getAs(const MapType &m, const std::string &key, const T &def = T()) {
            auto it = m.find(key);
            if (it == m.end()) {
                return def;
            }
            try {
                return boost::lexical_cast<T>(it->second);
            } catch (...) {
            }
            return def;
        }

        class HttpRequest {
        public:
            typedef std::shared_ptr <HttpRequest> ptr;

            typedef std::map <std::string, std::string, CaseInsensitiveLess> MapType;

            HttpRequest(uint8_t version = 0x11, bool close = true);

            HttpMethod getMethod() const { return m_method; }

            uint8_t getVersion() const { return m_version; }

            const std::string &getPath() const { return m_path; }

            const std::string &getQuery() const { return m_query; }

            const std::string &getBody() const { return m_body; }

            const MapType &getHeaders() const { return m_headers; }

            const MapType &getParams() const { return m_params; }

            const MapType &getCookies() const { return m_cookies; }

            void setMethod(HttpMethod method) { m_method = method; }

            void setVersion(uint8_t version) { m_version = version; }

            void setPath(const std::string &path) { m_path = path; }

            void setQuery(const std::string &query) { m_query = query; }

            void setFragment(const std::string &fragment) { m_fragment = fragment; }

            void setBody(const std::string &body) { m_body = body; }

            void setHeaders(const MapType &headers) { m_headers = headers; }

            void setParams(const MapType &params) { m_params = params; }

            void setCookies(const MapType &params) { m_params = params; }

            std::string getHeader(const std::string &key, const std::string &def = "") const;

            std::string getParam(const std::string &key, const std::string &def = "") const;

            std::string getCookie(const std::string &key, const std::string &def = "") const;

            void setHeader(const std::string &key, const std::string &val);

            void setParam(const std::string &key, const std::string &val);

            void setCookie(const std::string &key, const std::string &val);

            void delHeader(const std::string &key);

            void delParam(const std::string& key);

            void delCookie(const std::string& key);

            bool hasHeader(const std::string &key, std::string *val = nullptr);

            bool hasParam(const std::string &key, std::string *val = nullptr);

            bool hasCookie(const std::string &key, std::string *val = nullptr);

            template<typename T>
            bool checkGetHeaderAs(const std::string &key, T &val, const T &def = T()) {
                return checkGetAs(m_headers, key, val, def);
            }

            template<typename T>
            T getHeaderAs(const std::string &key, const T &def = T()) {
                return getAs(m_headers, key, def);
            }

            template<typename T>
            bool checkGetParamAs(const std::string &key, T &val, const T &def = T()) {
                return checkGetAs(m_params, key, val, def);
            }

            template<typename T>
            T getParamAs(const std::string &key, const T &def = T()) {
                return getAs(m_params, key, def);
            }

            template<typename T>
            bool checkGetCookieAs(const std::string &key, T &val, const T &def = T()) {
                return checkGetAs(m_cookies, key, val, def);
            }

            template<typename T>
            T getCookieAs(const std::string &key, const T &def = T()) {
                return getAs(m_cookies, key, def);
            }

        private:
            bool m_close;
            bool m_webSocket;
            uint8_t m_status;

            HttpMethod m_method;
            uint8_t m_version;

            std::string m_url;

            std::string m_path;
            std::string m_query;
            std::string m_fragment;
            std::string m_body;

            MapType m_headers;
            MapType m_params;
            MapType m_cookies;
        }
    }
};
#endif //LUWU_HTTP_H
