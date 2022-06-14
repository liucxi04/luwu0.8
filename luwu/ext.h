//
// Created by liucxi on 2022/6/14.
//

#ifndef LUWU_EXT_H
#define LUWU_EXT_H

#include <string>

namespace liucxi {

    /**
     * @brief 日期时间转字符串
    */
    std::string Time2Str(time_t ts = time(nullptr), const std::string &format = "%Y-%m-%d %H:%M:%S");

    class StringUtil {
    public:
        /**
         * @brief printf风格的字符串格式化，返回格式化后的string
         */
        static std::string Format(const char *fmt, ...);

        /**
         * @brief vprintf风格的字符串格式化，返回格式化后的string
         */
        static std::string Formatv(const char *fmt, va_list ap);

        /**
         * @brief url编码
         * @param[in] str 原始字符串
         * @param[in] space_as_plus 是否将空格编码成+号，如果为false，则空格编码成%20
         * @return 编码后的字符串
         */
        static std::string UrlEncode(const std::string &str, bool space_as_plus = true);

        /**
         * @brief url解码
         * @param[in] str url字符串
         * @param[in] space_as_plus 是否将+号解码为空格
         * @return 解析后的字符串
         */
        static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

        /**
         * @brief 移除字符串首尾的指定字符串
         * @param[] str 输入字符串
         * @param[] delimit 待移除的字符串
         * @return  移除后的字符串
         */
        static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");

        /**
         * @brief 移除字符串首部的指定字符串
         * @param[] str 输入字符串
         * @param[] delimit 待移除的字符串
         * @return  移除后的字符串
         */
        static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");

        /**
         * @brief 移除字符尾部的指定字符串
         * @param[] str 输入字符串
         * @param[] delimit 待移除的字符串
         * @return  移除后的字符串
         */
        static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

        /**
         * @brief 宽字符串转字符串
         */
        static std::string WStringToString(const std::wstring &ws);

        /**
         * @brief 字符串转宽字符串
         */
        static std::wstring StringToWString(const std::string &s);

    };
}
#endif //LUWU_EXT_H
