//
// Created by liucxi on 2022/4/9.
//

#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

using namespace liucxi;

ConfigVar<int>::ptr g_int = Config::lookup("system.port", (int)8080, "sys port");
ConfigVar<float>::ptr g_float = Config::lookup("system.value", (float)12.3f, "sys value");

void test_config() {
    std::cout << "g_int value: " << g_int->getValue() << std::endl;
    std::cout << "g_float value: " << Config::lookup<float>("system.value")->toString() << std::endl;

    YAML::Node root = YAML::LoadFile("/home/liucxi/Documents/luwu/tests/log.yml");
//    std::cout << root << std::endl;
    Config::loadFromYaml(root);

    std::cout << "g_int value: " << g_int->getValue() << std::endl;
    std::cout << "g_float value: " << g_float->toString() << std::endl;


}
int main(int argc, char *argv[]) {

    test_config();
    return 0;
}

