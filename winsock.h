// winsock.h - Windows socket class
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2def.h>
#include <array>
#include <compare>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

namespace winsock {

	// internet address
	enum class INADDR : ULONG {
		ANY = INADDR_ANY,
		LOOPBACK = INADDR_LOOPBACK,
		BROADCAST = INADDR_BROADCAST,
		NONE = INADDR_NONE,
	};

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
	enum class IPPROTO : int {
		HOPOPTS = IPPROTO_HOPOPTS,
		ICMP = IPPROTO_ICMP,
		IGMP = IPPROTO_IGMP,
		GGP = IPPROTO_GGP,
		IPV4 = IPPROTO_IPV4,
		ST = IPPROTO_ST,
		TCP = IPPROTO_TCP,
		CBT = IPPROTO_CBT,
		EGP = IPPROTO_EGP,
		IGP = IPPROTO_IGP,
		PUP = IPPROTO_PUP,
		UDP = IPPROTO_UDP,
		IDP = IPPROTO_IDP,
		RDP = IPPROTO_RDP,
		IPV6 = IPPROTO_IPV6,
		ROUTING = IPPROTO_ROUTING,
		FRAGMENT = IPPROTO_FRAGMENT,
		ESP = IPPROTO_ESP,
		AH = IPPROTO_AH,
		ICMPV6 = IPPROTO_ICMPV6,
		NONE = IPPROTO_NONE,
		DSTOPTS = IPPROTO_DSTOPTS,
		ND = IPPROTO_ND,
		ICLFXBM = IPPROTO_ICLFXBM,
		PIM = IPPROTO_PIM,
		PGM = IPPROTO_PGM,
		L2TP = IPPROTO_L2TP,
		SCTP = IPPROTO_SCTP,
		RAW = IPPROTO_RAW,
	};
	
	///  port IPPORT
	enum IPPORT {
		TCPMUX = IPPORT_TCPMUX,
		ECHO = IPPORT_ECHO,
		DISCARD = IPPORT_DISCARD,
		SYSTAT = IPPORT_SYSTAT,
		DAYTIME = IPPORT_DAYTIME,
		NETSTAT = IPPORT_NETSTAT,
		QOTD = IPPORT_QOTD,
		MSP = IPPORT_MSP,
		CHARGEN = IPPORT_CHARGEN,
		FTP_DATA = IPPORT_FTP_DATA,
		FTP = IPPORT_FTP,
		TELNET = IPPORT_TELNET,
		SMTP = IPPORT_SMTP,
		TIMESERVER = IPPORT_TIMESERVER,
		NAMESERVER = IPPORT_NAMESERVER,
		WHOIS = IPPORT_WHOIS,
		MTP = IPPORT_MTP,
		TFTP = IPPORT_TFTP,
		RJE = IPPORT_RJE,
		FINGER = IPPORT_FINGER,
		TTYLINK = IPPORT_TTYLINK,
		SUPDUP = IPPORT_SUPDUP,
		POP3 = IPPORT_POP3,
		NTP = IPPORT_NTP,
		EPMAP = IPPORT_EPMAP,
		NETBIOS_NS = IPPORT_NETBIOS_NS,
		NETBIOS_DGM = IPPORT_NETBIOS_DGM,
		NETBIOS_SSN = IPPORT_NETBIOS_SSN,
		IMAP = IPPORT_IMAP,
		SNMP = IPPORT_SNMP,
		SNMP_TRAP = IPPORT_SNMP_TRAP,
		IMAP3 = IPPORT_IMAP3,
		LDAP = IPPORT_LDAP,
		HTTP = 80, // not in w2def.h???
		HTTPS = IPPORT_HTTPS,
		MICROSOFT_DS = IPPORT_MICROSOFT_DS,
		EXECSERVER = IPPORT_EXECSERVER,
		LOGINSERVER = IPPORT_LOGINSERVER,
		CMDSERVER = IPPORT_CMDSERVER,
		EFSSERVER = IPPORT_EFSSERVER,
		BIFFUDP = IPPORT_BIFFUDP,
		WHOSERVER = IPPORT_WHOSERVER,
		ROUTESERVER = IPPORT_ROUTESERVER,
	};
	
	/// send/recv flags
	enum class MSG : int {
		DEFAULT = 0,
		DONTROUTE = MSG_DONTROUTE,
		OOB = MSG_OOB, // both send and recv
		PEEK = MSG_PEEK,
		WAITALL = MSG_WAITALL,
	};

#ifdef ERROR
#define ERROR_FOOBAR ERROR
#undef ERROR
#endif

	// getsockopt(SOL_SOCKET, SO_...)
#define GET_SOL_SOCKET(X) \
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

#ifdef ERROR_FOOBAR
#define ERROR ERROR_FOOBAR
#endif

#define SET_SOL_SOCKET(X) \
	X(BROADCAST, BOOL, "Configures a socket for sending broadcast data.") \
	X(CONDITIONAL_ACCEPT, BOOL, "Enables incoming connections are to be accepted or rejected by the application, not by the protocol stack.") \
	X(DEBUG, BOOL, "Enables debug output. Microsoft providers currently do not output any debug information.") \
	X(DONTLINGER, BOOL, "Does not block close waiting for unsent data to be sent. Setting this option is equivalent to setting SO_LINGER with l_onoff set to zero.") \
	X(DONTROUTE, BOOL, "Sets whether outgoing data should be sent on interface the socket is bound to and not a routed on some other interface. This option is not supported on ATM sockets (results in an error).") \
	X(GROUP_PRIORITY, int, "Reserved.") \
	X(KEEPALIVE, BOOL, "Enables sending keep-alive packets for a socket connection. Not supported on ATM sockets (results in an error).") \
	X(OOBINLINE, BOOL, "Indicates that out-of-bound data should be returned in-line with regular data. This option is only valid for connection-oriented protocols that support out-of-band data. For a discussion of this topic, see Protocol Independent Out-Of-band Data.") \
	X(RCVBUF, int, "Specifies the total per-socket buffer space reserved for receives.") \
	X(REUSEADDR, BOOL, "Allows the socket to be bound to an address that is already in use. For more information, see bind. Not applicable on ATM sockets.") \
	X(EXCLUSIVEADDRUSE, BOOL, "Enables a socket to be bound for exclusive access. Does not require administrative privilege.") \
	X(RCVTIMEO, DWORD, "Sets the timeout, in milliseconds, for blocking receive calls.") \
	X(SNDBUF, int, "Specifies the total per-socket buffer space reserved for sends.") \
	X(SNDTIMEO, DWORD, "The timeout, in milliseconds, for blocking send calls.") \

//	X(SO_UPDATE_ACCEPT_CONTEXT, int, "Updates the accepting socket with the context of the listening socket.") \
//	X(LINGER, LINGER, "Lingers on close if unsent data is present.") \

#define SO_ENUM(name, type, desc) name = (SO_ ## name),
	enum class GET_SO : int {
		GET_SOL_SOCKET(SO_ENUM)
	};
	enum SET_SO {
		SET_SOL_SOCKET(SO_ENUM)
	};
#undef SO_ENUM
	
	template<enum GET_SO T> struct get_sol_socket_type { };
#define GET_SO_SOCKET_TYPE(name, T, desc) template<> struct get_sol_socket_type<GET_SO::##name> { typedef T type; };
	GET_SOL_SOCKET(GET_SO_SOCKET_TYPE)
#undef GET_SO_SOCKET_TYPE

	template<enum SET_SO T> struct set_sol_socket_type { };
#define SET_SO_SOCKET_TYPE(name, T, desc) template<> struct set_sol_socket_type<SET_SO::##name> { typedef T type; };
	SET_SOL_SOCKET(SET_SO_SOCKET_TYPE)
#undef SET_SO_SOCKET_TYPE

	/// Get socket options of known type.
	template<enum GET_SO type>
	inline typename get_sol_socket_type<type>::type sockopt(SOCKET s) 
	{
		typename get_sol_socket_type<type>::type t;
		int len(sizeof(t));

		::getsockopt(s, SOL_SOCKET, static_cast<int>(type), (char*)&t, &len);

		return t;
	}
	/// Set socket options of known type.
	template<enum SET_SO type>
	inline int sockopt(SOCKET s, typename set_sol_socket_type<type>::type t)
	{
		return ::setsockopt(s, SOL_SOCKET, static_cast<int>(type), (char*)&t, sizeof(t));
	}
	
	//!!! inline ... linger(SOCKET s, ...)

	// addrinfo ai_flags for getaddrinfo
#define AI_ENUM(X) \
	X(PASSIVE, "Setting the AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function. When the AI_PASSIVE flag is set and pNodeName is a NULL pointer, the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses and IN6ADDR_ANY_INIT for IPv6 addresses.") \
	X(CANONNAME, "If neither AI_CANONNAME nor AI_NUMERICHOST is used, the getaddrinfo function attempts resolution. If a literal string is passed getaddrinfo attempts to convert the string, and if a host name is passed the getaddrinfo function attempts to resolve the name to an address or multiple addresses.") \
	X(NUMERICHOST, "When the AI_NUMERICHOST bit is set, the pNodeName parameter must contain a non-NULL numeric host address string, otherwise the EAI_NONAME error is returned. This flag prevents a name resolution service from being called.") \
	X(NUMERICSERV, "When the AI_NUMERICSERV bit is set, the pServiceName parameter must contain a non-NULL numeric port number, otherwise the EAI_NONAME error is returned. This flag prevents a name resolution service from being called.") \
	X(ADDRCONFIG, "If the AI_ADDRCONFIG bit is set, getaddrinfo will resolve only if a global address is configured. If AI_ADDRCONFIG flag is specified, IPv4 addresses shall be returned only if an IPv4 address is configured on the local system, and IPv6 addresses shall be returned only if an IPv6 address is configured on the local system. The IPv4 or IPv6 loopback address is not considered a valid global address.") \
	X(V4MAPPED, "If the AI_V4MAPPED bit is set and a request for IPv6 addresses fails, a name service request is made for IPv4 addresses and these addresses are converted to IPv4-mapped IPv6 address format.") \
	X(NON_AUTHORITATIVE, "If the AI_NON_AUTHORITATIVE bit is set, the NS_EMAIL namespace provider returns both authoritative and non-authoritative results. If the AI_NON_AUTHORITATIVE bit is not set, the NS_EMAIL namespace provider returns only authoritative results.") \
	X(SECURE, "If the AI_SECURE bit is set, the NS_EMAIL namespace provider will return results that were obtained with enhanced security to minimize possible spoofing.") \
	X(RETURN_PREFERRED_NAMES, "If the AI_RETURN_PREFERRED_NAMES is set, then no name should be provided in the pNodeName parameter. The NS_EMAIL namespace provider will return preferred names for publication.") \
	X(FQDN, "If the AI_FQDN is set and a flat name (single label) is specified, getaddrinfo will return the fully qualified domain name that the name eventually resolved to. The fully qualified domain name is returned in the ai_canonname member in the associated addrinfo structure. This is different than AI_CANONNAME bit flag that returns the canonical name registered in DNS which may be different than the fully qualified domain name that the flat name resolved to. Only one of the AI_FQDN and AI_CANONNAME bits can be set. The getaddrinfo function will fail if both flags are present with EAI_BADFLAGS.") \
	X(FILESERVER, "If the AI_FILESERVER is set, this is a hint to the namespace provider that the hostname being queried is being used in file share scenario. The namespace provider may ignore this hint.") \

#define X(a,b) a = AI_ ## a,
	enum class AI : int {
		DEFAULT = 0,
		AI_ENUM(X)
	};
#undef X

	///  Initialize winsock.
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
	
	// construct value object from dotted IPv4 or IPv6 address string
	template<AF af = AF::INET>
	class sockaddr {
		std::array<char, sizeof(::sockaddr)> sa;
		int len_;
	public:
		static AF family()
		{
			return af;
		}
		sockaddr(int len = static_cast<int>(sizeof(::sockaddr)))
			: sa{}, len_(len)
		{ 
			sin().sin_family = static_cast<int>(af);
		}
		sockaddr(INADDR addr, int port)
			: sockaddr()
		{
			sin().sin_addr.s_addr = static_cast<int>(addr);
			sin().sin_port = htons(port);
		}
		sockaddr(const char* host, int port)
			: sockaddr()
		{
			int ret = inet_pton(static_cast<int>(af), host, (void*)&sin().sin_addr.s_addr);
			if (ret != 1) {
				//!!! use std::system_error and WSAGetLastError()
				throw std::runtime_error("inet_pton failed");
			}
			sin().sin_port = htons(port);
		}
		sockaddr(const sockaddr&) = default;
		sockaddr& operator=(const sockaddr&) = default;
		sockaddr(sockaddr&&) = default;
		sockaddr& operator=(sockaddr&&) = default;
		~sockaddr()
		{ }

		::sockaddr_in& sin()
		{
			return *(::sockaddr_in*)sa.data();
		}

		::sockaddr* operator&()
		{
			return (::sockaddr*)sa.data();
		}
		const ::sockaddr* operator&() const
		{
			return (const ::sockaddr*)sa.data();
		}

		int& len()
		{
			return len_;
		}
		int len() const
		{
			return len_;
		}
	};

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

	template<AF af = AF::INET>
	class addrinfo {
		::addrinfo* pai;
	public:
		addrinfo(::addrinfo* pai = nullptr)
			: pai(pai)
		{ }
		addrinfo(PCSTR host, PCSTR port, const ::addrinfo& hints)
		{
			// The getaddrinfo function provides protocol-independent translation from an ANSI host name to an address.
			int ret = ::getaddrinfo(host, port, &hints, &pai);
			if (0 != ret) {
				throw std::runtime_error(gai_strerrorA(ret)); //???lifetime
			}
			if (!pai) {
				throw std::runtime_error("getaddrinfo found no addresses");
			}
			if (pai->ai_family != static_cast<int>(af)) {
				throw std::runtime_error("getaddrinfo incompatible address family");
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

		/// Return an addrinfo to use as hints
		static ::addrinfo hints(SOCK type = SOCK::STREAM, IPPROTO proto = IPPROTO::TCP, AI flags = AI::DEFAULT)
		{
			::addrinfo ai;
			memset(&ai, 0, sizeof(ai));
			ai.ai_family = static_cast<int>(af);
			ai.ai_socktype = static_cast<int>(type);
			ai.ai_protocol = static_cast<int>(proto);
			ai.ai_flags = static_cast<int>(flags);

			return ai;
		}

		::sockaddr* operator&()
		{
			return pai->ai_addr;
		}
		const ::sockaddr* operator&() const
		{
			return pai->ai_addr;
		}
		const auto addrlen() const
		{
			return pai->ai_addrlen;
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

	template<AF af = AF::INET>
	class socket {
		::SOCKET s;
	public:
		socket(::SOCKET s)
			: s(s)
		{ }
		socket(SOCK type = SOCK::STREAM, IPPROTO proto = IPPROTO::TCP)
			: s(INVALID_SOCKET)
		{
			s = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
		}
		socket(const socket&) = default;
		socket& operator=(const socket&) = default;
		~socket()
		{
			if (s != INVALID_SOCKET) {
				::closesocket(s);
			}
		}

		/// Usable as a SOCKET
		operator ::SOCKET()
		{
			return s;
		}

		/// Address of the peer to which a socket is connected.
		sockaddr<af> peername()
		{
			sockaddr<af> sa;

			::getpeername(s, &sa, &sa.len());

			return sa;
		}
		
		/// address family, socket type, protocol
		::addrinfo hints() const
		{
			WSAPROTOCOL_INFO wsapi;
			int len = sizeof(wsapi);
			::addrinfo ai;

			memset(&ai, 0, sizeof(ai));

			int result = ::getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&wsapi, &len);
			if (0 == result) {
				ai.ai_family = wsapi.iAddressFamily;
				ai.ai_socktype = wsapi.iSocketType;
				ai.ai_protocol = wsapi.iProtocol;
				ai.ai_flags = 0; //??? wsapi.dwProviderFlags;
			}

			return ai;
		}
		
		// server
		int bind(const ::sockaddr* addr, int len)
		{
			return ::bind(s, addr, len);
		}		
		int bind(const sockaddr<af>& sa)
		{
			return bind(&sa, sa.len());
		}
		
		SOCKET accept(::sockaddr* addr, int* len)
		{
			return ::accept(s, addr, len);
		}
		SOCKET accept(sockaddr<af>& sa)
		{
			return ::accept(s, &sa, &sa.len());
		}

		int listen(int backlog = SOMAXCONN)
		{
			return ::listen(s, backlog);
		}

		// client
		int connect(const ::sockaddr* addr, int len)
		{
			return ::connect(s, addr, len);
		}
		int connect(const sockaddr<af>& sa)
		{
			return ::connect(s, &sa, sa.len());
		}
		int connect(const addrinfo<af>& ai)
		{
			int result = SOCKET_ERROR;

			for (const auto [addr, len] : ai) {
				result = connect(addr, len);
				if (0 == result) {
					break;
				}
			}

			return result;
		}
		int connect(const char* host, const char* port)
		{
			return connect(addrinfo(host, port, hints()));
		}

		int send(const char* msg, int len = 0, MSG flags = MSG::DEFAULT)
		{
			if (0 == len) {
				len = static_cast<int>(strlen(msg));
			}

			for (int ret = 0, n = len; n; n -= ret) {
				ret = ::send(s, msg, len, static_cast<int>(flags));
				if (SOCKET_ERROR == ret) {
					return ret;
				}
			}

			return len;
		}
		socket& operator<<(std::istream& is)
		{
			int sndbuf = sockopt<GET_SO::SNDBUF>(s);
			std::vector<char> snd(sndbuf);
			while (is.read(snd.data(), sndbuf)) {
				send(snd.data(), static_cast<int>(is.gcount()));
				if (is.eof()) {
					break;
				}
			}

			return *this;
		}

		int recv(char* buf, int len, MSG flags = MSG::DEFAULT)
		{
			return ::recv(s, buf, len, static_cast<int>(flags));
		}
		std::vector<char> recv(MSG flags = MSG::DEFAULT)
		{
			int rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			std::vector<char> rcv(rcvbuf);

			int ret = 0, off = 0;
			while (0 != (ret = recv(rcv.data() + off, rcvbuf, flags))) {
				// if (flags == MSG::OOB) {}
				if (SOCKET_ERROR == ret) {
					// int err = WSAGetLastError();
					rcv.resize(0);

					break;
				}
				else if (ret == rcvbuf) {
					// more data to be read
					rcv.resize(rcv.size() + rcvbuf);
					off += rcvbuf;
				}
				else {
					// assert(ret < rcvbuf);
					rcv.erase(rcv.begin() + off + ret, rcv.end());

					break;
				}
			}

			return rcv;
		}
		//??? no copy using stream_buf
		socket& operator>>(std::ostream& os)
		{
			int rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			std::vector<char> rcv(rcvbuf);
			int ret = 0;
			while (0 < (ret = recv(rcv.data(), rcvbuf))) {
				os.write(rcv.data(), ret);
				if (ret < rcvbuf) {
					break; //??? check for pending data
				}
			}
			// if (ret == SOCKET_ERROR) ...

			return *this;
		}

		int sendto(const char* buf, int len, MSG flags, const ::sockaddr* to, int tolen) 
		{
			return ::sendto(s, buf, len, static_cast<int>(flags), to, tolen);
		}
		int recvfrom(char* buf, int len, MSG flags, ::sockaddr* in, int* inlen)
		{
			return ::recvfrom(s, buf, len, static_cast<int>(flags), in, inlen);
		}

	};

	// Specialize default values for constructor and member functions.
	namespace tcp {
		namespace client {
			template<AF af = AF::INET>
			class socket {
				winsock::socket<af> s;
			public:
				// create and connect socket
				socket(const char* host, const char* port)
					: s(SOCK::STREAM, IPPROTO::TCP)
				{
					s.connect(host, port);
				}
			};
		}
	}
	
	// Specialize default values for constructor
	namespace udp {
		namespace client {
			template<AF af = AF::INET>
			class socket {
				winsock::socket<af> s;
			public:
				socket()
					: s(SOCK::DGRAM, IPPROTO::UDP)
				{ }
				operator ::SOCKET()
				{
					return s;
				}
				int sendto(const sockaddr<af>& sa, char* buf, int len, MSG flags = MSG::DEFAULT)
				{
					return s.sendto(buf, len, flags, &sa, sa.len());
				}
				int recvfrom(sockaddr<af>& sa, char* buf, int len, MSG flags = MSG::DEFAULT)
				{
					return s.recvfrom(buf, len, flags, &sa, &sa.len());
				}
			};
		}
		namespace server {
			template<AF af = AF::INET>
			class socket {
				winsock::socket<af> s;
			public:
				// create and bind the udp socket
				socket(const sockaddr<af>& sa)
					: s(SOCK::DGRAM, IPPROTO::UDP)
				{
					s.bind(sa);
				}
				sockaddr<af> recvfrom(char* buf, int len, MSG flags = MSG::WAITALL)
				{
					sockaddr<af> sa;

					int n = s.recvfrom(buf, len, flags, &sa, &len);
					if (n > 0 && n < len) {
						buf[n] = 0;
					}

					return sa;
				}
				int sendto(const sockaddr<af>& sa, const char* buf, int len, MSG flags = MSG::DEFAULT) //???MSG::CONFIRM
				{
					return s.sendto(buf, len, flags, &sa, sa.len());
				}
			};
		}
	}
	
	/// Define bitwise operators.
	DEFINE_ENUM_FLAG_OPERATORS(MSG);
	DEFINE_ENUM_FLAG_OPERATORS(AI);
}
