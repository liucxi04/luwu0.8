//
// Created by liucxi on 2022/4/9.
//

#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

liucxi::ConfigVar<int>::ptr g_int =
        liucxi::Config::lookup("sys.port", (int)8080, "sys port");
void test_yaml() {
    YAML::Node root = YAML::LoadFile("log.yml");
    LUWU_LOG_INFO(LUWU_LOG_ROOT()) << root;
}
int main(int argc, char *argv[]) {
    std::cout << g_int->getValue();
    std::cout << g_int->toString();
    test_yaml();
    return 0;
}

