#pragma once

#include <string>

//! UDP socket.
class UdpSocket {
public:
    //! Constructor.
    UdpSocket();
    //! Destructor.
    ~UdpSocket();

    //! Connects the socket to the given host.
    //! Opens both a port for reading and writing.
    bool connect(const std::string &host, int portRead, int portWrite);

    //! Disconnects the socket.
    void disconnect();

    //! Writes a datagram.
    //! Returns the number of bytes sent.
    int write(const unsigned char *data, int len);

    //! Reads a datagram.
    //! Returns the number of bytes received, zero if no datagram is available.
    int read(unsigned char *data, int len);

protected:
    int _socket;
    long _remoteAddress;
    int _portWrite;
};
