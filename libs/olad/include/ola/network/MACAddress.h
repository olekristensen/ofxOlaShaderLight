/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * MACAddress.h
 * Represents a MAC Address
 * Copyright (C) 2013 Peter Newman
 */

#ifndef INCLUDE_OLA_NETWORK_MACADDRESS_H_
#define INCLUDE_OLA_NETWORK_MACADDRESS_H_

/**
 * @addtogroup network
 * @{
 * @file MACAddress.h
 * @brief Represents a MAC Address.
 * @}
 */

#ifdef WIN32
#include <winsock2.h>
// TODO(Peter): Do something else, possibly define the type locally
#else
#include <sys/types.h>  // required for FreeBSD uchar - doesn't hurt others
#include <net/ethernet.h>
#endif

#ifdef __FreeBSD__
// In the FreeBSD struct ether_addr, the single field is named octet, instead
// of ether_addr_octet.
// OS X does this too, but avoids it by adding the following line to its
// header, for compatibility with linux and others:
// http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/net/ethernet.h
#define ether_addr_octet octet
#endif

#include <stdint.h>
#include <string.h>
#include <sstream>
#include <string>

namespace ola {
namespace network {

/**
 * @addtogroup network
 * @{
 */

/**
 * @brief Represents a MAC Address.
 *
 * All methods use network byte order unless otherwise mentioned.
 * TODO(Peter): Is the above actually true for MAC addresses?
 */
class MACAddress {
 public:
    enum { LENGTH = ETHER_ADDR_LEN };

    MACAddress() {
      memset(m_address.ether_addr_octet, 0, LENGTH);
    }

    explicit MACAddress(const struct ether_addr &address)
        : m_address(address) {
    }

    /**
     * @brief Construct a new MAC address from binary data.
     * @param data a pointer to the memory containing the MAC address data. The
     * data should be most significant byte first.
     */
    explicit MACAddress(const uint8_t *data) {
      memcpy(m_address.ether_addr_octet, data, LENGTH);
    }

    MACAddress(const MACAddress &other)
        : m_address(other.m_address) {
    }

    MACAddress& operator=(const MACAddress &other) {
      if (this != &other) {
        m_address = other.m_address;
      }
      return *this;
    }

    bool operator==(const MACAddress &other) const {
      return (memcmp(m_address.ether_addr_octet,
                     other.m_address.ether_addr_octet,
                     LENGTH) == 0);
    }

    bool operator!=(const MACAddress &other) const {
      return !(*this == other);
    }

    /**
     * @brief Order addresses. Note that this won't order how humans expect
     * because ether_addr is in network byte order.
     * TODO(Peter): Check if this is actually true for MAC Addresses
     */
    bool operator<(const MACAddress &other) const {
      return (memcmp(m_address.ether_addr_octet,
                     other.m_address.ether_addr_octet,
                     LENGTH) < 0);
    }

    bool operator>(const MACAddress &other) const {
      return (memcmp(m_address.ether_addr_octet,
                     other.m_address.ether_addr_octet,
                     LENGTH) > 0);
    }

    const struct ether_addr& Address() const {
      return m_address;
    }

    // copy the address in network byte order to a location. The location
    // should be at least LENGTH bytes.
    void Get(uint8_t ptr[LENGTH]) const {
      memcpy(ptr,
             reinterpret_cast<const uint8_t*>(&m_address),
             LENGTH);
    }

    /**
     * @brief Write the binary representation of the MAC address to memory.
     * @param buffer a pointer to memory to write the MAC address to
     * @param length the size of the memory block, should be at least LENGTH.
     * @returns true if length was >= LENGTH, false otherwise.
     */
    bool Pack(uint8_t *buffer, unsigned int length) const {
      if (length < LENGTH)
        return false;
      Get(buffer);
      return true;
    }


    std::string ToString() const;

    friend std::ostream& operator<< (std::ostream &out,
                                     const MACAddress &address) {
      return out << address.ToString();
    }

    /**
     * Convert a string to a MACAddress object
     * @param address a string in the form 'nn:nn:nn:nn:nn:nn' or
     * 'nn.nn.nn.nn.nn.nn'
     * @return a pointer to a MACAddress object if it worked, NULL otherwise
     */
    static MACAddress* FromString(const std::string &address);

    /**
     * Convert a string to a MACAddress object
     * @param address a string in the form 'nn:nn:nn:nn:nn:nn' or
     * 'nn.nn.nn.nn.nn.nn'
     * @param[out] target a pointer to a MACAddress object
     * @return true if it worked, false otherwise
     */
    static bool FromString(const std::string &address, MACAddress *target);

    // useful for testing
    static MACAddress FromStringOrDie(const std::string &address);

 private:
    struct ether_addr m_address;
};
/**
 * @}
 */
}  // namespace network
}  // namespace ola
#endif  // INCLUDE_OLA_NETWORK_MACADDRESS_H_
