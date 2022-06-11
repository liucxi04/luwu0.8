//
// Created by liucxi on 2022/6/11.
//
#include "http/http_server.h"

void run() {
    liucxi::http::HttpServer::ptr server(new liucxi::http::HttpServer);
    liucxi::Address::ptr address = liucxi::Address::LookupAny("0.0.0.0:1234");
    while (!server->bind(address)) {
        sleep(2);
    }
    server->start();
}



int main() {
    liucxi::IOManager iom;
    iom.scheduler(run);
    return 0;
}
