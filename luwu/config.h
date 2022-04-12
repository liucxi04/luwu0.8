//
// Created by liucxi on 2022/4/9.
//

#ifndef LUWU_CONFIG_H
#define LUWU_CONFIG_H

#include <string>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <list>
#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>
#include "log.h"

namespace liucxi {

    /**
     * @brief 配置类基类，其核心功能需要子类实现
     * */
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        explicit ConfigVarBase(std::string name, std::string describe = "")
                : m_name(std::move(name)), m_describe(std::move(describe)) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        virtual ~ConfigVarBase() = default;

        const std::string &getName() const { return m_name; }

        const std::string &getDescribe() const { return m_describe; }

        virtual std::string toString() = 0;

        virtual bool fromString(const std::string &val) = 0;

        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;         /// 配置名称
        std::string m_describe;     /// 配置描述
    };

    /**
     * @brief 类型转换
     * */
    template<typename From, typename To>
    class LexicalCast {
    public:
        To operator()(const From &v) {
            return boost::lexical_cast<To>(v);
        }
    };

    /**
     * 模板偏特化，string to vector<T>，下同
     * */
    template<typename To>
    class LexicalCast<std::string, std::vector<To>> {
    public:
        std::vector<To> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::vector<To> vec;
            std::stringstream ss;
            for (auto i: node) {
                ss.str("");
                ss << i;
                vec.push_back(LexicalCast<std::string, To>()(ss.str()));
            }
            return vec;
        }
    };

    template<typename From>
    class LexicalCast<std::vector<From>, std::string> {
    public:
        std::string operator()(const std::vector<From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i: v) {
                node.push_back(YAML::Load(LexicalCast<From, std::string>()(i)));
            }
            ss << node;
            return ss.str();
        }
    };

    template<typename To>
    class LexicalCast<std::string, std::set<To>> {
    public:
        std::set<To> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::set<To> vec;
            std::stringstream ss;
            for (auto i: node) {
                ss.str("");
                ss << i;
                vec.insert(LexicalCast<std::string, To>()(ss.str()));
            }
            return vec;
        }
    };

    template<typename From>
    class LexicalCast<std::set<From>, std::string> {
    public:
        std::string operator()(const std::set<From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i: v) {
                node.push_back(YAML::Load(LexicalCast<From, std::string>()(i)));
            }
            ss << node;
            return ss.str();
        }
    };

    template<typename To>
    class LexicalCast<std::string, std::map<std::string, To>> {
    public:
        std::map<std::string, To> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, To> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(),
                                          LexicalCast<std::string, To>()(ss.str())));
            }
            return vec;
        }
    };

    template<typename From>
    class LexicalCast<std::map<std::string, From>, std::string> {
    public:
        std::string operator()(const std::map<std::string, From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i: v) {
                node[i.first] = YAML::Load(LexicalCast<From, std::string>()(i.second));
            }
            ss << node;
            return ss.str();
        }
    };


    /**
     * @brief 配置基类的实现类
     * */
    template<typename T, typename FromStr = LexicalCast<std::string, T>, typename ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar<T>> ptr;
        typedef std::function<void( const T &old_value, const T &new_value)>  on_change;

        ConfigVar(const std::string &name, const T &default_val, const std::string &describe)
                : ConfigVarBase(name, describe), m_val(default_val) {
        }

        /**
         * @brief 将 T 类型的值转为字符串
         * */
        std::string toString() override {
            try {
                return ToStr()(getValue());
            } catch (std::exception &e) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ConfigVar::toString exception " << e.what()
                                                << " convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

        /**
         * @brief 将字符串转为 T 类型的值
         * */
        bool fromString(const std::string &val) override {
            try {
                setValue(FromStr()(val));
            } catch (std::exception &e) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ConfigVar::fromString exception " << e.what()
                                                << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        T getValue() const { return m_val; }

        void setValue(const T &val) {
            if (val == m_val) {
                return;
            }
            for (auto &i : m_callbacks) {
                i.second(m_val, val);
            }
            m_val = val;
        }

        std::string getTypeName() const override { return typeid(T).name(); }

        void addListener(uint64_t key, on_change callback) {
            m_callbacks[key] = callback;
        }

        void delListener(uint64_t key) {
            m_callbacks.erase(key);
        }

        on_change getListener(uint64_t key) {
            auto it = m_callbacks.find(key);
            return it == m_callbacks.end() ? nullptr : it->second;
        }

        void clearListener() {
            m_callbacks.clear();
        }

    private:
        T m_val;
        /**
         * @brief 变更回调函数
         * */
        std::map<uint64_t, on_change> m_callbacks;
    };

    /**
     * @brief 配置管理类，单例模式
     * */
    class Config {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        /**
         * @brief 添加配置名和配置值
         * */
        template<typename T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name, const T &default_val,
                                                 const std::string &description = "") {
            /// 这种方法会出错
            /*
            auto tmp = lookup<T>(name);
            if (tmp) {
                LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "lookup name = " << name << "exists";
                return tmp;
            }*/
            auto it = s_data.find(name);
            if (it != s_data.end()) {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if (tmp) {
                    return tmp;
                } else {
                    LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "lookup name = " << name << " exists but type not "
                                                    << typeid(T).name() << ", real type = " << it->second->getTypeName()
                                                    << " " << it->second->toString();
                }
            }
            if (name.find_first_not_of("qwertyuiopasdfghjklzxcvbnm._0123456789") != std::string::npos) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "lookup name invalid" << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_val, description));
            s_data[name] = v;
            return v;
        }

        /**
         * @brief 按照配置名查找配置值
         * */
        template<typename T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name) {
            auto it = s_data.find(name);
            if (it == s_data.end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        static void loadFromYaml(const YAML::Node &root);

        static ConfigVarBase::ptr lookupBase(const std::string &name);

    private:
        static ConfigVarMap s_data;
    };

}
#endif //LUWU_CONFIG_H
