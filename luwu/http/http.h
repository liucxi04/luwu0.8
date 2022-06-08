//
// Created by liucxi on 2022/6/6.
//

#ifndef LUWU_HTTP_H
#define LUWU_HTTP_H

#include <memory>
#include <string>
#include <map>

#include "http-parser/http_parser.h"
#include <boost/lexical_cast.hpp>

namespace liucxi {
    namespace http {

        enum class HttpMethod {
#define XX(num, name, string) name = num,
            HTTP_METHOD_MAP(XX)
#undef XX
            INVALID_METHOD
        };

        enum class HttpStatus {
#define XX(code, name, desc) name = code,
            HTTP_STATUS_MAP(XX)
#undef XX
        };

        static HttpMethod StringToHttpMethod(const std::string &m);

        static HttpMethod CharsToHttpMethod(const char *m);

        static std::string HttpMethodToString(const HttpMethod &m);

        static std::string HttpStatusToString(const HttpStatus &s);

        struct CaseInsensitiveLess {
            bool operator()(const std::string &lhs, const std::string &rhs) const;
        };

        template<typename MapType, typename T>
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

        template<typename MapType, typename T>
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
            typedef std::shared_ptr<HttpRequest> ptr;

            typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

            explicit HttpRequest(uint8_t version = 0x11, bool close = true);

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

            void appendBody(const std::string &body) { m_body.append(body); }

            void setHeaders(const MapType &headers) { m_headers = headers; }

            void setParams(const MapType &params) { m_params = params; }

            void setCookies(const MapType &params) { m_params = params; }

            bool isClose() const { return m_close; }

            void setClose(bool close) { m_close = close; }

            std::string getHeader(const std::string &key, const std::string &def = "") const;

            std::string getParam(const std::string &key, const std::string &def = "") const;

            std::string getCookie(const std::string &key, const std::string &def = "") const;

            void setHeader(const std::string &key, const std::string &val) { m_headers[key] = val; }

            void setParam(const std::string &key, const std::string &val) { m_params[key] = val; }

            void setCookie(const std::string &key, const std::string &val) { m_cookies[key] = val; }

            void delHeader(const std::string &key) { m_headers.erase(key); }

            void delParam(const std::string &key) { m_params.erase(key); }

            void delCookie(const std::string &key) { m_cookies.erase(key); }

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

            std::string toString() const;

            std::ostream &dump(std::ostream &os) const;

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
        };

        class HttpResponse {
        public:
            typedef std::shared_ptr<HttpResponse> ptr;
            typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

            explicit HttpResponse(uint8_t version = 0x11, bool close = true);

            HttpStatus getStatus() const { return m_status; }

            uint8_t getVersion() const { return m_version; }

            const std::string &getBody() const { return m_body; }

            const std::string &getReason() const { return m_reason; }

            const MapType &getHeaders() const { return m_headers; }

            void setStatus(HttpStatus status) { m_status = status; }

            void setVersion(uint8_t version) { m_version = version; }

            void setBody(const std::string &body) { m_body = body; }

            void appendBody(const std::string &body) { m_body.append(body); }

            void setReason(const std::string &reason) { m_reason = reason; }

            void setHeaders(const MapType &header) { m_headers = header; }

            bool isClose() const { return m_close; }

            void setClose(bool v) { m_close = v; }

            bool isWebSocket() const { return m_webSocket; }

            void setWebSocket(bool webSocket) { m_webSocket = webSocket; }

            std::string getHeader(const std::string& key, const std::string& def = "") const;

            void setHeader(const std::string& key, const std::string& val) { m_headers[key] = val; }

            void delHeader(const std::string& key) { m_headers.erase(key); }

            template<class T>
            bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
                return checkGetAs(m_headers, key, val, def);
            }

            template<class T>
            T getHeaderAs(const std::string& key, const T& def = T()) {
                return getAs(m_headers, key, def);
            }

            std::ostream& dump(std::ostream& os) const;

            std::string toString() const;

        private:
            HttpStatus m_status;
            uint8_t m_version;
            bool m_close;
            bool m_webSocket;
            std::string m_body;
            std::string m_reason;
            MapType m_headers;
            std::vector<std::string> m_cookies;
        };
    }
}
#endif //LUWU_HTTP_H
