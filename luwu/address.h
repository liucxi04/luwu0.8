//
// Created by liucxi on 2022/6/1.
//

#ifndef LUWU_ADDRESS_H
#define LUWU_ADDRESS_H

#include <memory>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>

namespace liucxi {
    class Address {
    public:
        typedef std::shared_ptr<Address> ptr;

        ~Address() = default;

        int getFamily() const;

        virtual const sockaddr *getAddr() const = 0;

        virtual socklen_t getAddrLen() const = 0;

        virtual std::ostream &insert(std::ostream &os) const = 0;

        std::string toString() const;

        bool operator<(const Address &rhs) const;

        bool operator==(const Address &rhs) const;

        bool operator!=(const Address &rhs) const;
    };

    class IPAddress : public Address {
    public:
        typedef std::shared_ptr<IPAddress> ptr;

        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

        virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;

        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

        virtual uint16_t getPort() const = 0;

        virtual void setPort(uint16_t p) = 0;
    };

    class IPv4Address : public IPAddress {
    public:
        typedef std::shared_ptr<IPv4Address> ptr;

        explicit IPv4Address(const sockaddr_in &addr);

        explicit IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        const sockaddr *getAddr() const override;

        socklen_t getAddrLen() const override;

        std::ostream &insert(std::ostream &os) const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        IPAddress::ptr networkAddress(uint32_t prefix_len) override;

        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint16_t getPort() const override;

        void setPort(uint16_t p) override;

    private:
        sockaddr_in m_addr{};
    };

    class IPv6Address : public IPAddress {
    public:
        typedef std::shared_ptr<IPv6Address> ptr;

        IPv6Address();

        IPv6Address(const sockaddr_in6 &addr);

        explicit IPv6Address(const char *address, uint16_t port = 0);

        const sockaddr *getAddr() const override;

        socklen_t getAddrLen() const override;

        std::ostream &insert(std::ostream &os) const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        IPAddress::ptr networkAddress(uint32_t prefix_len) override;

        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint16_t getPort() const override;

        void setPort(uint16_t p) override;

    private:
        sockaddr_in6 m_addr{};
    };

    class UnixAddress : public Address {
    public:
        typedef std::shared_ptr<UnixAddress> ptr;

        UnixAddress();

        explicit UnixAddress(std::string &path);

        const sockaddr *getAddr() const override;

        socklen_t getAddrLen() const override;

        std::ostream &insert(std::ostream &os) const override;

    private:
        sockaddr_un m_addr{};
        socklen_t m_length{};
    };

    class UnknownAddress : public Address {
    public:
        typedef std::shared_ptr<UnknownAddress> ptr;

        explicit UnknownAddress(int family);

        const sockaddr *getAddr() const override;

        socklen_t getAddrLen() const override;

        std::ostream &insert(std::ostream &os) const override;

    private:
        sockaddr m_addr{};
    };
}
#endif //LUWU_ADDRESS_H
