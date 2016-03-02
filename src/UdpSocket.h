#pragma once

#include <string>

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    bool connect(const std::string &host, int portRead, int portWrite);
    void disconnect();
    int write(const unsigned char *data, int len);
    int read(unsigned char *data, int len);

protected:
    int _socket;
    long _remoteAddress;
    int _portWrite;
};
