//
// Created by liucxi on 2022/4/9.
//

#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

using namespace liucxi;

ConfigVar<int>::ptr g_int = Config::lookup("system.port", (int)8080, "sys port");

ConfigVar<float>::ptr g_float = Config::lookup("system.port", (float)12.3f, "sys port");
ConfigVar<std::vector<int>>::ptr g_vector = Config::lookup("system.vector",
                                                           std::vector<int>{1,2},"system vector");
ConfigVar<std::set<int>>::ptr g_set = Config::lookup("system.set",
                                                           std::set<int>{1,2},"system set");
ConfigVar<std::map<std::string, int>>::ptr g_map = Config::lookup("system.map",
                                                           std::map<std::string , int>{{"k", 2}},"system map");

void test_config() {
    std::cout << "g_int value: " << g_int->getValue() << std::endl;
    std::cout << "g_float value: " << g_float->toString() << std::endl;
    auto v = g_vector->getValue();
    for (auto i : v) {
        std::cout << "g_vector value: " << i << std::endl;
    }
    auto z = g_set->getValue();
    for (auto i : z) {
        std::cout << "g_set value: " << i << std::endl;
    }
    auto x = g_map->getValue();
    for (const auto& i : x) {
        std::cout << "g_map value: " << i.first << " " << i.second << std::endl;
    }


    YAML::Node root = YAML::LoadFile("/home/liucxi/Documents/luwu/tests/log.yml");
//    std::cout << root << std::endl;
    Config::loadFromYaml(root);

    std::cout << "g_int value: " << g_int->getValue() << std::endl;
    std::cout << "g_float value: " << g_float->toString() << std::endl;
    v = g_vector->getValue();
    for (auto i : v) {
        std::cout << "g_vector value: " << i << std::endl;
    }
    z = g_set->getValue();
    for (auto i : z) {
        std::cout << "g_set value: " << i << std::endl;
    }
    x = g_map->getValue();
    for (const auto& i : x) {
        std::cout << "g_map value: " << i.first << " " << i.second << std::endl;
    }
}

class Person {
public:
    std::string m_name;
    int m_age = 0;
    bool m_sex = false;
    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person &oth) const {
        return m_name == oth.m_name;
    }
};

namespace liucxi {
    template<>
    class LexicalCast<std::string, Person> {
    public:
        Person operator() (const std::string &v) {
            YAML::Node node = YAML::Load(v);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<int>();
            return p;
        }
    };

    template<>
    class LexicalCast<Person, std::string> {
    public:
        std::string operator() (const Person &p) {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

ConfigVar<Person>::ptr g_person = Config::lookup("class.person", Person(), "class person");

ConfigVar<std::vector<Person>>::ptr g_vec_person = Config::lookup("class.vec_person", std::vector<Person>(), "");

void test_class() {

    g_person->addListener(10, [](const Person &old_val, const Person &new_val){
        std::cout << old_val.toString() << new_val.toString() << std::endl;
    });
    std::cout << "g_person value: " << g_person->toString() << std::endl;
    std::cout << "g_vec_person value: " << g_vec_person->toString() << std::endl;

    YAML::Node root = YAML::LoadFile("/home/liucxi/Documents/luwu/tests/log.yml");
    Config::loadFromYaml(root);

    std::cout << "g_person value: " << g_person->toString() << std::endl;
    std::cout << "g_vec_person value: " << g_vec_person->toString() << std::endl;

}
int main(int argc, char *argv[]) {

//     test_config();
    test_class();
    return 0;
}

