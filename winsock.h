// winsock.h - Windows socket class
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <compare>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#pragma comment(lib, "Ws2_32.lib")

namespace win {

	/// address family
	enum class AF : int {
		UNSPEC = AF_UNSPEC,
		UNIX = AF_UNIX, // file name
		INET = AF_INET, // internet address
		IMPLINK = AF_IMPLINK,
		PUP = AF_PUP,
		CHAOS = AF_CHAOS,
		NS = AF_NS,
		IPX = AF_IPX,
		ISO = AF_ISO,
		OSI = AF_OSI,
		ECMA = AF_ECMA,
		DATAKIT = AF_DATAKIT,
		CCITT = AF_CCITT,
		SNA = AF_SNA,
		DECnet = AF_DECnet,
		DLI = AF_DLI,
		LAT = AF_LAT,
		HYLINK = AF_HYLINK,
		APPLETALK = AF_APPLETALK,
		NETBIOS = AF_NETBIOS,
		VOICEVIEW = AF_VOICEVIEW,
		FIREFOX = AF_FIREFOX,
		UNKNOWN1 = AF_UNKNOWN1,
		BAN = AF_BAN,
		ATM = AF_ATM,
		INET6 = AF_INET6,
		CLUSTER = AF_CLUSTER,
		_12844 = AF_12844,
		IRDA = AF_IRDA,
	};

	/// socket type
	enum class SOCK : int {
		STREAM = SOCK_STREAM, // tcp char stream
		DGRAM = SOCK_DGRAM,   // udp message packets
		RAW = SOCK_RAW,
		RDM = SOCK_RDM,
		SEQPACKET = SOCK_SEQPACKET,
	};

	class WSA {
		WSADATA wsaData;
	public:
		WSA()
		{
			if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
				throw std::runtime_error("WSAStartup failed");
			}
		}
		WSA(const WSA&) = delete;
		WSA& operator=(const WSA&) = delete;
		~WSA()
		{
			WSACleanup();
		}
	};
	static inline const WSA wsa;

	/// forward iterator over addrinfo pointers
	class addrinfo_iter {
		::addrinfo* pai;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<::sockaddr*, int>; // ai_addr and ai_addr_len

		addrinfo_iter(::addrinfo* pai = nullptr)
			: pai(pai)
		{ }

		auto operator<=>(const addrinfo_iter&) const = default;

		value_type operator*() const
		{
			return value_type(pai->ai_addr, static_cast<int>(pai->ai_addrlen));
		}
		addrinfo_iter& operator++()
		{
			if (pai) {
				pai = pai->ai_next;
			}

			return *this;
		}
	};


	class addrinfo {
		::addrinfo* pai;
	public:
		addrinfo(::addrinfo* pai = nullptr)
			: pai(pai)
		{ }
		addrinfo(PCSTR host, PCSTR port, const ::addrinfo& hints)
		{
			if (0 != ::getaddrinfo(host, port, &hints, &pai)) {
				throw std::runtime_error("getaddrinfo failed");
			}
		}
		addrinfo(const addrinfo&) = default;
		addrinfo& operator=(const addrinfo&) = default;
		~addrinfo()
		{
			if (pai) {
				freeaddrinfo(pai);
			}
		}

		addrinfo_iter begin() const
		{
			return addrinfo_iter(pai);
		}
		addrinfo_iter end() const
		{
			return addrinfo_iter();
		}
	};

	class socket {
		::SOCKET s;
	public:
		socket(::SOCKET s)
			: s(s)
		{ }
		socket(AF family = AF::UNSPEC, SOCK socktype = SOCK::STREAM, IPPROTO protocol = IPPROTO_TCP)
			: s(INVALID_SOCKET)
		{
			s = ::socket(static_cast<int>(family), static_cast<int>(socktype), static_cast<int>(protocol));
		}
		socket(const socket&) = default;
		socket& operator=(const socket&) = default;
		~socket()
		{
			if (s != INVALID_SOCKET) {
				::closesocket(s);
			}
		}

		/// <summary>
		/// Use as a SOCKET
		/// </summary>
		operator ::SOCKET()
		{
			return s;
		}

		/// Address of the peer to which a socket is connected.
		std::pair<::sockaddr*, int> getpeername()
		{
			::sockaddr* name = nullptr;
			int len = 0;

			::getpeername(s, name, &len);

			return std::pair(name, len);
		}

		::addrinfo hints() const
		{
			::addrinfo ai;
			ZeroMemory(&ai, sizeof(ai));

			WSAPROTOCOL_INFO wsapi;
			int len = sizeof(wsapi);
			if (0 == ::getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&wsapi, &len)) {
				ai.ai_family = wsapi.iAddressFamily;
				ai.ai_socktype = wsapi.iSocketType;
				ai.ai_protocol = wsapi.iProtocol;
			}

			return ai;
		}

		// server
		int bind(const sockaddr* addr, int len)
		{
			return ::bind(s, addr, len);
		}
		SOCKET accept(sockaddr* addr, int* len)
		{
			return ::accept(s, addr, len);
		}
		int listen(int backlog)
		{
			return ::listen(s, backlog);
		}

		// client
		int connect(const sockaddr* addr, int len)
		{
			return ::connect(s, addr, len);
		}
		int connect(const char* host, const char* port)
		{
			int result = SOCKET_ERROR;

			for (const auto [addr, len] : addrinfo(host, port, hints())) {
				result = connect(addr, len);
				if (0 == result) {
					break;
				}
			}

			return result;
		}

		/// send/recv flags
		enum class MSG : int {
			DEFAULT   = 0,
			DONTROUTE = MSG_DONTROUTE,
			OOB       = MSG_OOB, // both send and recv
			PEEK      = MSG_PEEK, 
			WAITALL   = MSG_WAITALL,
		};

		int send(const char* msg, int len = 0, MSG flags = MSG::DEFAULT)
		{
			if (0 == len) {
				len = static_cast<int>(strlen(msg));
			}

			return ::send(s, msg, len, static_cast<int>(flags));
		}
		socket& operator<<(const char* s)
		{
			send(s);

			return *this;
		}
		socket& operator<<(std::istream& is)
		{
			while (!is.eof()) {
				char c;
				is >> c;
				send(&c, 1);
			}

			return *this;
		}
		int recv(char* buf, int len, MSG flags = MSG::DEFAULT)
		{
			return ::recv(s, buf, len, static_cast<int>(flags));
		}
		socket& operator>>(std::ostream& os)
		{
			char c = 0;
			while (1 == recv(&c, 1)) {
				os << c;
			}

			return *this;
		}

		int sendto(const char* buf, int len, int flags, const sockaddr* to, int tolen) 
		{
			return ::sendto(s, buf, len, flags, to, tolen);
		}
		int recvfrom(char* buf, int len, int flags, sockaddr* in, int* inlen)
		{
			return ::recvfrom(s, buf, len, flags, in, inlen);
		}

	};
	DEFINE_ENUM_FLAG_OPERATORS(socket::MSG)
}
