#include "UdpSocket.h"

#ifdef WIN32
  #include <winsock2.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <unistd.h>
#endif

#include <fcntl.h>

UdpSocket::UdpSocket() :
    _socket(-1)
{
}

UdpSocket::~UdpSocket()
{
    disconnect();
}

bool UdpSocket::connect(const std::string &host, int portRead, int portWrite)
{
    struct sockaddr_in hostAddr;
    struct hostent *hostInfo;

    if (_socket != -1)
        return false;

    _portWrite = portWrite;

#ifdef WIN32
    // Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
    }
#endif

    // Create socket
    if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        _socket = -1;
#ifdef WIN32
		WSACleanup();
#endif
        return false;
    }

    // Resolve hostname
    hostInfo = gethostbyname(host.c_str());
	if (hostInfo == nullptr) {
#ifdef WIN32
		WSACleanup();
#endif
		return false;
	}

	// Get remote address
    memcpy(&_remoteAddress, hostInfo->h_addr, hostInfo->h_length);

    hostAddr.sin_addr.s_addr=INADDR_ANY;
    hostAddr.sin_port=htons(portRead);
    hostAddr.sin_family=AF_INET;

    if (bind(_socket, reinterpret_cast<struct sockaddr *>(&hostAddr), sizeof(hostAddr)) < 0) {
        disconnect();
#ifdef WIN32
		WSACleanup();
#endif
        return false;
    }

    // Make non-blocking
#ifdef WIN32
	u_long iMode = 1; 
	if (ioctlsocket(_socket, FIONBIO, &iMode) != 0) {
		WSACleanup();
		return false;
	}
#else
    int status;
    if ((status=fcntl(_socket, F_GETFL)) < 0) {
        disconnect();
        return false;
    }

    status |= O_NONBLOCK;

    if (fcntl(_socket, F_SETFL, status) < 0) {
        disconnect();
        return false;
    }
#endif

    return true;
}

//==============================================================================
void UdpSocket::disconnect(void)
{
    if (_socket == -1) {
        return;
    }

#ifdef WIN32
    closesocket(_socket);
	WSACleanup();
#else
    close(_socket);
#endif
    
    _socket = -1;
}

int UdpSocket::write(const unsigned char *data, int len)
{
    if (_socket < 0) {
        return 0;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = _remoteAddress;
    addr.sin_port        = htons(_portWrite);
    addr.sin_family      = AF_INET;

    return sendto(_socket, data, len, 0, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr));
}


int UdpSocket::read(unsigned char *data, int len)
{
    if (_socket < 0) {
        return 0;
    }

    int received = recv(_socket, data, len, 0);
    return (received < 0) ? 0 : received;
}
