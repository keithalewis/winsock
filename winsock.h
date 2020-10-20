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

namespace winsock {

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

	/// protocol IPPROTO

#ifdef ERROR
#define ERROR_ ERROR
#undef ERROR
#endif
	// getsockopt(SOL_SOCKET, ...)
#define SOL_SOCKET_ARGS(X) \
	X(ACCEPTCONN, BOOL, "The socket is listening.") \
	X(BROADCAST, BOOL, "The socket is configured for the transmission and receipt of broadcast messages.") \
	X(BSP_STATE, CSADDR_INFO, "Returns the local address, local port, remote address, remote port, socket type, and protocol used by a socket.") \
	X(CONDITIONAL_ACCEPT, BOOL, "Returns current socket state, either from a previous call to setsockopt or the system default.") \
	X(DEBUG, BOOL, "Debugging is enabled.") \
	X(DONTLINGER, BOOL, "If TRUE, the LINGER option is disabled.") \
	X(DONTROUTE, BOOL, "Routing is disabled. Setting this succeeds but is ignored on AF_INET sockets; fails on AF_INET6 sockets with WSAENOPROTOOPT. This option is not supported on ATM sockets.") \
	X(ERROR, int, "Retrieves error status and clear.") \
	X(EXCLUSIVEADDRUSE, BOOL, "Prevents any other socket from binding to the same address and port. This option must be set before calling the bind function.") \
	X(GROUP_ID, GROUP, "Reserved.") \
	X(GROUP_PRIORITY, int, "Reserved.") \
	X(KEEPALIVE, BOOL, "Keep-alives are being sent. Not supported on ATM sockets.") \
	X(MAX_MSG_SIZE, unsigned int, "The maximum size of a message for message-oriented socket types (for example, SOCK_DGRAM). Has no meaning for stream oriented sockets.") \
	X(OOBINLINE, BOOL, "OOB data is being received in the normal data stream. (See section Windows Sockets 1.1 Blocking Routines and EINPROGRESS for a discussion of this topic.)") \
	X(PORT_SCALABILITY, BOOL, "Enables local port scalability for a socket by allowing port allocation to be maximized by allocating wildcard ports multiple times for different local address port pairs on a local machine.") \
	X(PROTOCOL_INFO, WSAPROTOCOL_INFO, "A description of the protocol information for the protocol that is bound to this socket.") \
	X(RCVBUF, int, "The total per-socket buffer space reserved for receives. This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.") \
	X(REUSEADDR, BOOL, "The socket can be bound to an address which is already in use. Not applicable for ATM sockets.") \
	X(SNDBUF, int, "The total per-socket buffer space reserved for sends. This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.") \
	X(TYPE, int, "The type of the socket (for example, SOCK_STREAM).") \

//	X(CONNECT_TIME, DWORD, "Returns the number of seconds a socket has been connected. This socket option is valid for connection oriented protocols only.") \
//	X(LINGER, LINGER, "Returns the current linger options.") \

#ifdef ERROR_
#define ERROR ERROR_
#endif

#define SO_ENUM(name, type, desc) name = (SO_ ## name), // desc
	enum SO {
		SOL_SOCKET_ARGS(SO_ENUM)
	};
#undef SO_ENUM
	
	template<int T> struct sol_socket_type { };
#define SO_SOCKET_TYPE(name, T, desc) template<> struct sol_socket_type<SO::##name> { typedef T type; };
	SOL_SOCKET_ARGS(SO_SOCKET_TYPE)
#undef SO_SOCKET_TYPE

	template<int T>
	inline typename sol_socket_type<T>::type getsockopt(SOCKET s) 
	{
		typename sol_socket_type<T>::type t;
		int len(sizeof(t));

		::getsockopt(s, SOL_SOCKET, T, (char*)&t, &len);

		return t;
	}

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

		/// address family, socket type, protocol
		int hints(::addrinfo* pai) const
		{
			WSAPROTOCOL_INFO wsapi;
			int len = sizeof(wsapi);

			int result = ::getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&wsapi, &len);
			if (0 == result) {
				pai->ai_family = wsapi.iAddressFamily;
				pai->ai_socktype = wsapi.iSocketType;
				pai->ai_protocol = wsapi.iProtocol;
			}

			return result;
		}
		::addrinfo hints() const
		{
			::addrinfo ai;
			ZeroMemory(&ai, sizeof(ai));

			hints(&ai);

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
