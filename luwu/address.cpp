//
// Created by liucxi on 2022/6/1.
//
#include "address.h"
#include "utils.h"

#include <memory>
#include <sstream>

namespace liucxi {

    template<typename T>
    static T CreateMask(uint32_t bits) {
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    int Address::getFamily() const {
        return getAddr()->sa_family;
    }

    std::string Address::toString() const {
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }

    bool Address::operator<(const Address &rhs) const {
        socklen_t minLen = std::min(getAddrLen(), rhs.getAddrLen());
        int res = memcmp(getAddr(), rhs.getAddr(), minLen);
        if (res < 0) {
            return true;
        } else if (res > 0) {
            return false;
        } else {
            if (getAddrLen() < rhs.getAddrLen()) {
                return true;
            } else {
                return false;
            }
        }
    }

    bool Address::operator==(const Address &rhs) const {
        return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen());
    }

    bool Address::operator!=(const Address &rhs) const {
        return !(*this == rhs);
    }

    IPv4Address::IPv4Address(const sockaddr_in &addr) {
        m_addr = addr;
    }

    IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = byteSwapOnLittleEndian(port);
        m_addr.sin_addr.s_addr = byteSwapOnLittleEndian(address);
    }

    const sockaddr *IPv4Address::getAddr() const {
        return (sockaddr *) &m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream &IPv4Address::insert(std::ostream &os) const {
        uint32_t address = byteSwapOnLittleEndian(m_addr.sin_addr.s_addr);
        os << ((address >> 24) & 0xff) << "."
           << ((address >> 16) & 0xff) << "."
           << ((address >> 8) & 0xff) << "."
           << (address & 0xff);
        os << ":" << byteSwapOnLittleEndian(m_addr.sin_port);
        return os;
    }

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }

        sockaddr_in addr(m_addr);
        addr.sin_addr.s_addr |= byteSwapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return std::make_shared<IPv4Address>(addr);
    }

    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }

        sockaddr_in addr(m_addr);
        addr.sin_addr.s_addr &= byteSwapOnLittleEndian(~CreateMask<uint32_t>(prefix_len));
        return std::make_shared<IPv4Address>(addr);
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
        if (prefix_len > 32) {
            return nullptr;
        }

        sockaddr_in addr{};
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = ~byteSwapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return std::make_shared<IPv4Address>(addr);
    }

    uint16_t IPv4Address::getPort() const {
        return byteSwapOnLittleEndian(m_addr.sin_port);
    }

    void IPv4Address::setPort(uint16_t p) {
        m_addr.sin_port = byteSwapOnLittleEndian(p);
    }

    IPv6Address::IPv6Address() {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
    }

    IPv6Address::IPv6Address(const sockaddr_in6 &addr) {
        m_addr = addr;
    }

    IPv6Address::IPv6Address(const char *address, uint16_t port) {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = byteSwapOnLittleEndian(port);
        memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
    }

    const sockaddr *IPv6Address::getAddr() const {
        return (sockaddr *) &m_addr;
    }

    socklen_t IPv6Address::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream &IPv6Address::insert(std::ostream &os) const {
        os << "[";
        auto addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
        bool used_zeros = false;
        for (size_t i = 0; i < 8; ++i) {
            if (addr[i] == 0 && !used_zeros) {
                continue;
            }
            if (i && addr[i-1] == 0 && !used_zeros) {
                os << ":";
                used_zeros = true;
            }
            if (i) {
                os << ":";
            }
            os << std::hex << (int) byteSwapOnLittleEndian(addr[i]) << std::dec;
        }

        if (!used_zeros && addr[7] == 0) {
            os << "::";
        }
        os << "]:" << byteSwapOnLittleEndian(m_addr.sin6_port);
        return os;
    }

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
        sockaddr_in6 addr(m_addr);
        addr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
        for (unsigned int i = prefix_len / 8 + 1; i < 16; ++i) {
            addr.sin6_addr.s6_addr[i] = 0xff;
        }
        return std::make_shared<IPv6Address>(addr);
    }

    IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {
        sockaddr_in6 addr(m_addr);
        addr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
        for (unsigned int i = prefix_len / 8 + 1; i < 16; ++i) {
            addr.sin6_addr.s6_addr[i] = 0x00;
        }
        return std::make_shared<IPv6Address>(addr);
    }

    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
        sockaddr_in6 addr{};
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
        for (unsigned int i = 0; i < prefix_len / 8; ++i) {
            addr.sin6_addr.s6_addr[i] = 0xff;
        }
        return std::make_shared<IPv6Address>(addr);
    }

    uint16_t IPv6Address::getPort() const {
        return byteSwapOnLittleEndian(m_addr.sin6_port);
    }

    void IPv6Address::setPort(uint16_t p) {
        m_addr.sin6_port = byteSwapOnLittleEndian(p);
    }

    static const size_t MAX_PATH_LENGTH = sizeof(((sockaddr_un*)nullptr)->sun_path) - 1;

    UnixAddress::UnixAddress() {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LENGTH;
    }

    UnixAddress::UnixAddress(std::string &path) {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = path.size() + 1;

        if (!path.empty() && path[0] == '\0') {
            --m_length;
        }

        if (m_length > sizeof(m_addr.sun_path)) {
            throw std::logic_error("path too long");
        }

        memcpy(m_addr.sun_path, path.c_str(), m_length);
        m_length += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr *UnixAddress::getAddr() const {
        return (sockaddr*)&m_addr;
    }

    socklen_t UnixAddress::getAddrLen() const {
        return m_length;
    }

    std::ostream &UnixAddress::insert(std::ostream &os) const {
        if (m_length > offsetof(sockaddr_un, sun_path)
                && m_addr.sun_path[0] == '\0') {
            return os << "\\0" << std::string(m_addr.sun_path + 1,
                                              m_length - offsetof(sockaddr_un, sun_path) - 1);
        }
        return os << m_addr.sun_path;
    }

    UnknownAddress::UnknownAddress(int family) {
        memset(&m_addr, 0 , sizeof(m_addr));
        m_addr.sa_family = family;
    }

    const sockaddr *UnknownAddress::getAddr() const {
        return &m_addr;
    }

    socklen_t UnknownAddress::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream &UnknownAddress::insert(std::ostream &os) const {
        os << "[UnknownAddress family=" << m_addr.sa_family << "]";
        return os;
    }
}
