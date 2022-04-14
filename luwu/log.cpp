//
// Created by liucxi on 2022/4/6.
//

#include <iostream>
#include <functional>
#include <cstdarg>
#include <set>
#include "log.h"
#include "config.h"

namespace liucxi {

    std::string LogLevel::toString(LogLevel::Level level) {
        switch (level) {
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARN:
                return "WARN";
            case ERROR:
                return "ERROR";
            case FATAL:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    }

    LogLevel::Level LogLevel::fromString(const std::string &str) {
#define XX(level, v) if (str == #v) { return LogLevel::level; }
        XX(DEBUG, DEBUG)
        XX(INFO, INFO)
        XX(WARN, WARN)
        XX(ERROR, ERROR)
        XX(FATAL, FATAL)
#undef XX
        return LogLevel::UNKNOWN;
    }

    LogEvent::LogEvent(std::string logger_name, LogLevel::Level level, const char *file, int32_t line,
                       int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time,
                       std::string thread_name)
            : m_loggerName(std::move(logger_name)), m_level(level), m_file(file), m_line(line), m_elapse(elapse),
              m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_threadName(std::move(thread_name)) {
    }

    /**
     * TODO Understand
     * */
    void LogEvent::printf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }

    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        explicit MessageFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LevelFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << LogLevel::toString(event->getLevel());
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        explicit ElapseFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class LoggerNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LoggerNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getLoggerName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        explicit ThreadIdFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        explicit FiberIdFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit ThreadNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        explicit DateTimeFormatItem(std::string format = "%Y-%m-%d %H:%M:%S")
                : m_format(std::move(format)) {
            if (m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, LogEvent::ptr event) override {
            struct tm tm{};
            auto time = (time_t) event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit FileNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LineFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        explicit NewLineFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        explicit TabFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << "\t";
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        explicit StringFormatItem(std::string str)
                : m_string(std::move(str)) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    LogFormatter::LogFormatter(std::string pattern)
            : m_pattern(std::move(pattern)) {
        init();
    }

    /**
     * @brief 状态机来实现字符串的解析
     * */
    void LogFormatter::init() {
        // 按顺序存储解析到的 pattern 项
        // 每个 pattern 包括一个整数类型和一个字符串，类型为 0 表示该 pattern 是常规字符串，为 1 表示该 pattern 需要转义
        // 日期格式单独用下面的 data_format 存储
        std::vector<std::pair<int, std::string>> patterns;
        // 临时存储常规字符串
        std::string tmp;
        // 日期格式字符串，默认把 %d 后面的内容全部当作格式字符串，不校验是否合法
        std::string data_format;
        // 是否解析出错
        bool error = false;
        // 正在解析常规字符串
        bool parsing_string = true;

        size_t i = 0;
        while (i < m_pattern.size()) {
            std::string c = std::string(1, m_pattern[i]);

            if (c == "%") {
                if (parsing_string) {
                    if (!tmp.empty()) {
                        patterns.emplace_back(0, tmp); // 在解析常规字符时遇到 %，表示开始解析模板字符
                    }
                    tmp.clear();
                    parsing_string = false;
                    ++i;
                    continue;
                }
                patterns.emplace_back(1, c); // 在解析模板字符时遇到 %，表示这里是一个转义字符
                parsing_string = true;
                ++i;
                continue;
            } else {
                if (parsing_string) {
                    tmp += c;
                    ++i;
                    continue;
                }
                patterns.emplace_back(1, c); // 模板字符直接添加，因为模板字符只有 1 个字母
                parsing_string = true;
                if (c != "d") {
                    ++i;
                    continue;
                }

                // 下面是对 %d 的特殊处理，直接取出 { } 内内容
                ++i;
                if (i < m_pattern.size() && m_pattern[i] != '{') {
                    continue; //不符合规范，不是 {
                }
                ++i;
                while (i < m_pattern.size() && m_pattern[i] != '}') {
                    data_format.push_back(m_pattern[i]);
                    ++i;
                }
                if (m_pattern[i] != '}') {
                    std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] '{' not closed"
                              << std::endl;
                    error = true;
                    break; //不符合规范，不是 }
                }
                ++i;
                continue;
            }
        } // end while
        if (error) {
            m_error = true;
            return;
        }
        // 模板解析最后的常规字符也要记得加进去
        if (!tmp.empty()) {
            patterns.emplace_back(0, tmp);
            tmp.clear();
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string)>> s_format_items = {
#define XX(str, C) { #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } },

                XX(m, MessageFormatItem)
                XX(p, LevelFormatItem)
                XX(c, LoggerNameFormatItem)
                XX(d, DateTimeFormatItem)
                XX(r, ElapseFormatItem)
                XX(f, FileNameFormatItem)
                XX(l, LineFormatItem)
                XX(t, ThreadIdFormatItem)
                XX(b, FiberIdFormatItem)
                XX(n, ThreadNameFormatItem)
                XX(N, NewLineFormatItem)
                XX(T, TabFormatItem)
#undef XX
        };

        for (const auto &v: patterns) {
            if (v.first == 0) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(v.second)));
            } else if (v.second == "d") {
                m_items.push_back(FormatItem::ptr(new DateTimeFormatItem(data_format)));
            } else {
                auto it = s_format_items.find(v.second);
                if (it == s_format_items.end()) {
                    std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] " <<
                              "unknown format item: " << v.second << std::endl;
                    error = true;
                    break;
                } else {
                    m_items.push_back(it->second(v.second));
                }
            }
        }

        if (error) {
            m_error = true;
            return;
        }
    }

    std::string LogFormatter::format(const LogEvent::ptr &event) {
        std::stringstream ss;
        for (const auto &i: m_items) {
            i->format(ss, event);
        }
        return ss.str();
    }

    std::ostream &LogFormatter::format(std::ostream &os, const LogEvent::ptr &event) {
        for (const auto &i: m_items) {
            i->format(os, event);
        }
        return os;
    }

    void StdoutLogAppender::log(LogEvent::ptr event) {
        if (m_formatter) {
            m_formatter->format(std::cout, event);
        } else {
            m_defaultFormatter->format(std::cout, event);
        }
    }

    std::string StdoutLogAppender::toYamlString() {
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        node["pattern"] = m_formatter->getPattern();
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    FileLogAppender::FileLogAppender(std::string filename)
            : LogAppender(std::make_shared<LogFormatter>()), m_filename(std::move(filename)) {
        reopen();
        if (m_reopenError) {
            std::cout << "reopen file " << m_filename << " error" << std::endl;
        }
    }

    bool FileLogAppender::reopen() {
        if (m_filestream) {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        m_reopenError = !m_filestream;
        return !m_reopenError;
    }

    void FileLogAppender::log(LogEvent::ptr event) {
        if (m_formatter) {
            m_formatter->format(m_filestream, event);
        } else {
            m_defaultFormatter->format(m_filestream, event);
        }
    }

    std::string FileLogAppender::toYamlString() {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        node["pattern"] = m_formatter ? m_formatter->getPattern() : m_defaultFormatter->getPattern();
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void Logger::addAppender(const LogAppender::ptr &appender) {
        m_appenderList.push_back(appender);
    }

    void Logger::delAppender(const LogAppender::ptr &appender) {
        m_appenderList.remove(appender);
    }

    void Logger::clearAppenders() {
        m_appenderList.clear();
    }

    void Logger::log(const LogEvent::ptr &event) {
        if (event->getLevel() >= m_level) {
            for (const auto &i: m_appenderList) {
                i->log(event);
            }
        }
    }

    std::string Logger::toYamlString() {
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevel::toString(m_level);
        for (const auto &i : m_appenderList) {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    LoggerManager::LoggerManager() {
        m_root.reset(new Logger("root"));
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
        m_loggers[m_root->getName()] = m_root;
    }

    std::string LoggerManager::toYamlString() {
        YAML::Node node;
        for (const auto &i : m_loggers) {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name) {
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
            return it->second;
        }

        Logger::ptr logger(new Logger(name));
        m_loggers[name] = logger;
        return logger;
    }

    /// 从配置文件中加载日志配置
    /**
     * @brief 日志输出器配置结构体
     * */
    struct LogAppenderDefine {
        int type = 0; // 1 File, 2 Stdout
        std::string pattern;
        std::string file;

        bool operator==(const LogAppenderDefine &oth) const {
            return type == oth.type && pattern == oth.pattern && file == oth.file;
        }
    };

    /**
     * @brief 日志器配置结构体
     * */
    struct LoggerDefine {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LoggerDefine &oth) const {
            return name == oth.name && level == oth.level && appenders == oth.appenders;
        }

        bool operator<(const LoggerDefine &oth) const {
            return name < oth.name;
        }

        bool isValid() const {
            return !name.empty();
        }
    };

    template<>
    class LexicalCast<std::string, LoggerDefine> {
        LoggerDefine operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            LoggerDefine loggerDefine;

            if (!node["name"].IsDefined()) {
                std::cout << "log config error: name is null, " << node << std::endl;
                throw std::logic_error("log config name is null");
            }
            loggerDefine.name = node["name"].as<std::string>();
            loggerDefine.level = LogLevel::fromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");

            if (node["appenders"].IsDefined()) {
                for (const auto &appender : node["appenders"]) {
                    if (!appender["type"].IsDefined()) {
                        std::cout << "log config error: appender type is null, " << appender << std::endl;
                        continue;
                    }
                    std::string type = appender["type"].as<std::string>();
                    LogAppenderDefine logAppenderDefine;
                    if (type == "FileLogAppender") {
                        logAppenderDefine.type = 1;
                        if (!appender["file"].IsDefined()) {
                            std::cout << "log config error: appender file is null, " << appender << std::endl;
                        }
                        logAppenderDefine.file = appender["file"].as<std::string>();
                        if (appender["pattern"].IsDefined()) {
                            logAppenderDefine.pattern = appender["pattern"].as<std::string>();
                        }
                    } else if (type == "StdoutLogAppender") {
                        logAppenderDefine.type = 2;
                        if (appender["pattern"].IsDefined()) {
                            logAppenderDefine.pattern = appender["pattern"].as<std::string>();
                        }
                    } else {
                        std::cout << "log config error: appender type is invalid, " << appender << std::endl;
                    }
                    loggerDefine.appenders.push_back(logAppenderDefine);
                }
            }
            return loggerDefine;
        }
    };

    template<>
    class LexicalCast<LoggerDefine, std::string> {
        std::string operator()(const LoggerDefine &loggerDefine) {
            YAML::Node node;
            node["name"] = loggerDefine.name;
            node["level"] = loggerDefine.level;
            for (const auto &appender : loggerDefine.appenders) {
                YAML::Node appenderNode;
                if (appender.type == 1) {
                    appenderNode["type"] = "FileLogAppender";
                    appenderNode["file"] = appender.file;
                } else if (appender.type == 2) {
                    appenderNode["type"] = "StdoutLogAppender";
                } else if (!appender.pattern.empty()) {
                    appenderNode["pattern"] = appender.pattern;
                }
                node["appenders"].push_back(appenderNode);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    liucxi::ConfigVar<std::set<LoggerDefine>>::ptr g_logDefines =
            liucxi::Config::lookup("logs", std::set<LoggerDefine>(), "logs config");

    struct LogInit {
        LogInit() {
            g_logDefines->addListener(104, [](const std::set<LoggerDefine> &oldVal, const std::set<LoggerDefine> &newVal){
                LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "log config changed";
                for (const auto & val : newVal) {
                    auto it = oldVal.find(val);
                    liucxi::Logger::ptr logger;
                    if (it == oldVal.end()) {
                        // 新增
                        logger = LUWU_LOG_NAME(val.name);
                    } else {
                        if (!(val == *it)) {
                            // 修改
                            logger = LUWU_LOG_NAME(val.name);
                        } else {
                            continue;
                        }
                    }
                    logger->setLevel(val.level);
                    logger->clearAppenders();
                    for (const auto &appender : val.appenders) {
                        liucxi::LogAppender::ptr logAppender;
                        if (appender.type == 1) {
                            logAppender.reset(new liucxi::FileLogAppender(appender.file));
                        } else if (appender.type == 2) {
                            logAppender.reset(new liucxi::StdoutLogAppender);
                        }
                        if (!appender.pattern.empty()) {
                            logAppender->setFormatter(std::make_shared<LogFormatter>(appender.pattern));
                        } else {
                            logAppender->setFormatter(std::make_shared<LogFormatter>());
                        }
                        logger->addAppender(logAppender);
                    }
                }

                for (const auto &val : oldVal) {
                    auto it = newVal.find(val);
                    if (it == newVal.end()) {
                        auto logger = LUWU_LOG_NAME(val.name);
                        /**
                         * @todo 日志等级还是有问题
                         * */
                        logger->setLevel(liucxi::LogLevel::UNKNOWN);
                        logger->clearAppenders();
                    }
                }
            });
        }
    };

    static LogInit __log_init;
}
