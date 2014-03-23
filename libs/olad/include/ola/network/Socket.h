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
 * Socket.h
 * The Socket interfaces
 * Copyright (C) 2005-2009 Simon Newton
 *
 *  - UDPSocket, allows sending and receiving UDP datagrams
 *  - TCPSocket, this represents a TCP connection to a remote endpoint
 *
 * AcceptingSocket is the interface that defines sockets which can spawn new
 * ConnectedDescriptors. TCPAcceptingSocket is the only subclass and provides
 * the accept() functionality.
 */

#ifndef INCLUDE_OLA_NETWORK_SOCKET_H_
#define INCLUDE_OLA_NETWORK_SOCKET_H_

#include <stdint.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <ola/Callback.h>
#include <ola/io/Descriptor.h>
#include <ola/io/IOQueue.h>
#include <ola/network/IPV4Address.h>
#include <ola/network/SocketAddress.h>
#include <string>


namespace ola {
namespace network {


/*
 * The UDPSocketInterface.
 * This is done as an Interface so we can mock it out for testing.
 */
class UDPSocketInterface: public ola::io::BidirectionalFileDescriptor {
 public:
    UDPSocketInterface(): ola::io::BidirectionalFileDescriptor() {}
    ~UDPSocketInterface() {}
    virtual bool Init() = 0;
    virtual bool Bind(const IPV4SocketAddress &endpoint) = 0;

    // Deprecated. Do not use in new code.
    bool Bind(const IPV4Address &ip, unsigned short port) {
      return Bind(IPV4SocketAddress(ip, port));
    }

    virtual bool GetSocketAddress(IPV4SocketAddress *address) const = 0;

    virtual bool Close() = 0;
    virtual int ReadDescriptor() const = 0;
    virtual int WriteDescriptor() const = 0;

    virtual ssize_t SendTo(const uint8_t *buffer,
                           unsigned int size,
                           const IPV4Address &ip,
                           unsigned short port) const = 0;
    virtual ssize_t SendTo(const uint8_t *buffer,
                           unsigned int size,
                           const IPV4SocketAddress &dest) const = 0;
    virtual ssize_t SendTo(ola::io::IOVecInterface *data,
                           const IPV4Address &ip,
                           unsigned short port) const = 0;
    virtual ssize_t SendTo(ola::io::IOVecInterface *data,
                           const IPV4SocketAddress &dest) const = 0;

    virtual bool RecvFrom(uint8_t *buffer, ssize_t *data_read) const = 0;
    virtual bool RecvFrom(uint8_t *buffer,
                          ssize_t *data_read,
                          IPV4Address &source) const = 0;  // NOLINT
    virtual bool RecvFrom(uint8_t *buffer,
                          ssize_t *data_read,
                          IPV4Address &source,  // NOLINT
                          uint16_t &port) const = 0;  // NOLINT

    virtual bool EnableBroadcast() = 0;
    virtual bool SetMulticastInterface(const IPV4Address &iface) = 0;
    virtual bool JoinMulticast(const IPV4Address &iface,
                               const IPV4Address &group,
                               bool loop = false) = 0;
    virtual bool LeaveMulticast(const IPV4Address &iface,
                                const IPV4Address &group) = 0;
    virtual bool SetTos(uint8_t tos) = 0;

 private:
    UDPSocketInterface(const UDPSocketInterface &other);
    UDPSocketInterface& operator=(const UDPSocketInterface &other);
};


/*
 * A UDPSocket (non connected)
 */
class UDPSocket: public UDPSocketInterface {
 public:
    UDPSocket(): UDPSocketInterface(),
                 m_fd(ola::io::INVALID_DESCRIPTOR),
                 m_bound_to_port(false) {}
    ~UDPSocket() { Close(); }
    bool Init();
    bool Bind(const IPV4SocketAddress &endpoint);

    bool GetSocketAddress(IPV4SocketAddress *address) const;

    bool Close();
    int ReadDescriptor() const { return m_fd; }
    int WriteDescriptor() const { return m_fd; }
    ssize_t SendTo(const uint8_t *buffer,
                   unsigned int size,
                   const IPV4Address &ip,
                   unsigned short port) const;
    ssize_t SendTo(const uint8_t *buffer,
                   unsigned int size,
                   const IPV4SocketAddress &dest) const {
      return SendTo(buffer, size, dest.Host(), dest.Port());
    }
    ssize_t SendTo(ola::io::IOVecInterface *data,
                   const IPV4Address &ip,
                   unsigned short port) const;
    ssize_t SendTo(ola::io::IOVecInterface *data,
                   const IPV4SocketAddress &dest) const {
      return SendTo(data, dest.Host(), dest.Port());
    }

    bool RecvFrom(uint8_t *buffer, ssize_t *data_read) const;
    bool RecvFrom(uint8_t *buffer,
                  ssize_t *data_read,
                  IPV4Address &source) const;  // NOLINT
    bool RecvFrom(uint8_t *buffer,
                  ssize_t *data_read,
                  IPV4Address &source,  // NOLINT
                  uint16_t &port) const;  // NOLINT

    bool EnableBroadcast();
    bool SetMulticastInterface(const IPV4Address &iface);
    bool JoinMulticast(const IPV4Address &iface,
                       const IPV4Address &group,
                       bool loop = false);
    bool LeaveMulticast(const IPV4Address &iface,
                        const IPV4Address &group);

    bool SetTos(uint8_t tos);

 private:
    int m_fd;
    bool m_bound_to_port;
    UDPSocket(const UDPSocket &other);
    UDPSocket& operator=(const UDPSocket &other);
    bool _RecvFrom(uint8_t *buffer,
                   ssize_t *data_read,
                   struct sockaddr_in *source,
                   socklen_t *src_size) const;
};
}  // namespace network
}  // namespace ola
#endif  // INCLUDE_OLA_NETWORK_SOCKET_H_
