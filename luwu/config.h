//
// Created by liucxi on 2022/4/9.
//

#ifndef LUWU_CONFIG_H
#define LUWU_CONFIG_H

#include <string>
#include <memory>
#include <sstream>
#include "log.h"
#include <boost/lexical_cast.hpp>

namespace liucxi {
    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        ConfigVarBase(const std::string &name, const std::string &describe = "")
                : m_name(name), m_describe(describe) {
        }

        virtual ~ConfigVarBase() {};

        const std::string &getName() const { return m_name; }

        const std::string &getDescribe() const { return m_describe; }

        virtual std::string toString() = 0;

        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_describe;
    };

    template<typename T>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar<T>> ptr;

        ConfigVar(const std::string &name, const T &default_val, const std::string &describe)
            : ConfigVarBase(name, describe)
            , m_val(default_val) {
        }

        std::string toString() override {
            try {
                return boost::lexical_cast<std::string>(m_val);
            } catch (std::exception &e) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ConfigVar::toString exception" << e.what()
                << "convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }
        bool fromString(const std::string &val) override {
            try {
                m_val = boost::lexical_cast<T>(val);
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
