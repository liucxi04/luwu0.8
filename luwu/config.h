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
#include <map>
#include <set>
#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>
#include "log.h"

namespace liucxi {
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        explicit ConfigVarBase(std::string name, std::string describe = "")
                : m_name(std::move(name)), m_describe(std::move(describe)) {
        }

        virtual ~ConfigVarBase() = default;

        const std::string &getName() const { return m_name; }

        const std::string &getDescribe() const { return m_describe; }

        virtual std::string toString() = 0;

        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_describe;
    };

    template<typename From, typename To>
    class LexicalCast {
    public:
        To operator()(const From &v) {
            return boost::lexical_cast<To>(v);
        }
    };

    template<typename To>
    class LexicalCast<std::string, std::vector<To>> {
    public:
        std::vector<To> operator() (const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::vector<To> vec;
            std::stringstream ss;
            for (auto i : node) {
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
        std::string operator() (const std::vector<From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<From, std::string>()(i)));
            }
            ss << node;
            return ss.str();
        }
    };

    template<typename To>
    class LexicalCast<std::string, std::set<To>> {
    public:
        std::set<To> operator() (const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::set<To> vec;
            std::stringstream ss;
            for (auto i : node) {
                ss.str("");
                ss << i;
                vec.intsert(LexicalCast<std::string, To>()(ss.str()));
            }
            return vec;
        }
    };

    template<typename From>
    class LexicalCast<std::set<From>, std::string> {
    public:
        std::string operator() (const std::set<From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i : v) {
                node.push_back(YAML::Load(LexicalCast<From, std::string>()(i)));
            }
            ss << node;
            return ss.str();
        }
    };

    template<typename To>
    class LexicalCast<std::string, std::map<std::string, To>> {
    public:
        std::map<std::string, To> operator() (const std::string &v) {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, To> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.intsert(std::make_pair(it->first.Scalar(),
                                           LexicalCast<std::string, To>()(ss.str())));
            }
            return vec;
        }
    };

    template<typename From>
    class LexicalCast<std::map<std::string, From>, std::string> {
    public:
        std::string operator() (const std::map<std::string, From> &v) {
            YAML::Node node;
            std::stringstream ss;
            for (auto &i : v) {
                node[i.first] = YAML::Load(LexicalCast<From, std::string>()(i.second));
            }
            ss << node;
            return ss.str();
        }
    };


    template<typename T, typename FromStr = LexicalCast<std::string, T>
                        , typename ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar<T>> ptr;

        ConfigVar(const std::string &name, const T &default_val, const std::string &describe)
                : ConfigVarBase(name, describe), m_val(default_val) {
        }

        std::string toString() override {
            try {
//                return boost::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            } catch (std::exception &e) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ConfigVar::toString exception" << e.what()
                                                << "convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override {
            try {
//                m_val = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
            } catch (std::exception &e) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what()
                                                << "convert: string to" << typeid(m_val).name();
            }
            return false;
        }

        T getValue() const { return m_val; }

        void setValue(T val) { m_val = val; }

    private:
        T m_val;
    };

    class Config {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template<typename T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name, const T &default_val,
                                                 const std::string &description = "") {
            auto tmp = lookup<T>(name);
            if (tmp) {
                LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "lookup name = " << name << "exists";
                return tmp;
            }
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_val, description));
            s_data[name] = v;
            return v;
        }

        template<typename T>
        static typename ConfigVar<T>::ptr lookup(const std::string &name) {
            auto it = s_data.find(name);
            if (it == s_data.end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

    private:
        static ConfigVarMap s_data;
    };

}
#endif //LUWU_CONFIG_H
