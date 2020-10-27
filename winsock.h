// winsock.h - Windows socket class
#pragma once
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <array>
#include <compare>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include "winsock_buffer.h"
#include "winsock_enum.h"


#pragma comment(lib, "Ws2_32.lib")

namespace winsock {

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

	/// Value object from dotted IPv4 or IPv6 address string
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
		sockaddr(INADDR addr, u_short port)
			: sockaddr()
		{
			sin().sin_addr.s_addr = static_cast<int>(addr);
			sin().sin_port = htons(port);
		}
		sockaddr(const char* host, u_short port)
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
	

	/// Forward iterator over addrinfo pointers
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
		static ::addrinfo hints(SOCK type, IPPROTO proto, AI flags)
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
		//!!! Add functions to get info about packet size for address.
	};

	template<AF af = AF::INET>
	class socket {
		::SOCKET s;
		/*
		// closesocket will be called on s_
		static socket& assign(::SOCKET s_)
		{
			WSAPROTOCOL_INFO wsapi = sockopt<GET_SO::PROTOCOL_INFO>(s_);

			return *this;
		}
		*/
	public:
		socket(SOCK type, IPPROTO proto)
			: s(INVALID_SOCKET)
		{
			s = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
		}
		socket(const socket&) = delete;
		socket& operator=(const socket&) = delete;
		// may need to call shutdown first!
		~socket()
		{
			if (s != INVALID_SOCKET) {
				::closesocket(s);
			}
		}

		/// Usable as a SOCKET
		operator ::SOCKET() const
		{
			return s;
		}

		/// Address of the peer to which a socket is connected.
		sockaddr<af> peername() const
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
		
		//
		// server
		//
		// Establish the local association of the socket by assigning a local name to an unnamed socket.
		int bind(const ::sockaddr* addr, int len) const
		{
			return ::bind(s, addr, len);
		}		
		int bind(const sockaddr<af>& sa) const
		{
			return bind(&sa, sa.len());
		}
		int bind(const addrinfo<af>& ai) const
		{
			int result = SOCKET_ERROR;

			for (const auto [addr, len] : ai) {
				result = bind(addr, len);
				if (0 == result) {
					break;
				}
			}

			return result;
		}

		int listen(int backlog = SOMAXCONN) const
		{
			return ::listen(s, backlog);
		}

		::SOCKET accept(::sockaddr* addr, int* len) const
		{
			return ::accept(s, addr, len);
		}
		socket<af> accept(sockaddr<af>& sa) const
		{
			return assign(::accept(s, &sa, &sa.len()));
		}

		//
		// client
		//
		int connect(const ::sockaddr* addr, int len) const
		{
			return ::connect(s, addr, len);
		}
		int connect(const sockaddr<af>& sa) const
		{
			return ::connect(s, &sa, sa.len());
		}
		int connect(const addrinfo<af>& ai) const
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
		int connect(const char* host, const char* port) const
		{
			return connect(addrinfo(host, port, hints()));
		}

		int send(const char* msg, int len = 0, SNDMSG flags = SNDMSG::DEFAULT) const
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
		int send(const std::string& msg, SNDMSG flags = SNDMSG::DEFAULT) const
		{
			return send(msg.c_str(), msg.length(), flags);
		}
		/*
		int send(const buffer& msg, SNDMSG flags = SNDMSG::DEFAULT) const
		{
			return send(&msg, msg.len(), flags);
		}
		*/
		/// <summary>
		/// Send input stream to socket. 
		/// </summary>
		/// <param name="is">input stream</param>
		/// <param name="flags">input flags</param>
		/// <returns>Total number of characters written</returns>
		int send(std::istream& is, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			int len = 0;
			int sndbuf = sockopt<GET_SO::SNDBUF>(s);
			std::vector<char> snd(sndbuf);

			while (is.read(snd.data(), sndbuf)) {
				len += send(snd.data(), static_cast<int>(is.gcount()));
				if (is.eof()) {
					break;
				}
			}

			return len;
		}
		socket& operator<<(const char* msg)
		{
			send(msg);

			return *this;
		}

		// istream proxy
		class istream_proxy {
			SNDMSG flags;
		public:
			istream_proxy(const SNDMSG& flags)
				: flags(flags)
			{ }
			class send {
				const socket<af>& s;
				istream_proxy flags;
			public:
				send(const socket<af>& s, istream_proxy flags)
					: s(s), flags(flags)
				{ }
				send(const send&) = default;
				send& operator=(const send&) = default;
				send(send&&) = default;
				send& operator=(send&&) = default;
				~send()
				{ }

				send& operator<<(std::istream& is)
				{
					s.send(is, flags.flags);

					return *this;
				}
				send& operator<<(const char* msg)
				{
					s.send(msg, 0, flags.flags);

					return *this;
				}
				template<size_t N>
				send& operator<<(const char (&msg)[N])
				{
					s.send(msg, static_cast<int>(N), flags.flags);

					return *this;
				}
				send& operator<<(const std::string& msg)
				{
					s.send(msg, flags.flags);

					return *this;
				}
			};
		};
		// s << flags(SNDMSG::XXX) << is ...
		static istream_proxy flags(const SNDMSG& flags)
		{
			return istream_proxy(flags);
		}
		typename istream_proxy::send operator<<(const istream_proxy& flags)
		{
			return istream_proxy::send(*this, flags);
		}			
		socket& operator<<(std::istream& is)
		{
			//!!!find appropriate size for connection
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

		int recv(char* buf, int len, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			return ::recv(s, buf, len, static_cast<int>(flags));
		}
		/*
		int recv(buffer& buf, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			int len = buf.len();
			if (len == 0) {
				buf.len(sockopt<GET_SO::RCVBUF>(s));
				len = buf.len();
			}

			int off = 0;
			int ret = SOCKET_ERROR;
			while (0 < (ret = recv(&buf + off, len, flags))) {
				if (ret == len) {
					off += len;
					buf.len(buf.len() + len);
				}
				else {
					buf.len(buf.len() + ret);
					break;
				}
			}
			// if (ret == 0) check for pending data???

			return ret;
		}
		*/
		int recv(std::ostream& os, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			int rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			std::vector<char> rcv(rcvbuf, 0);
			int ret = 0;
			while (0 < (ret = recv(rcv.data(), rcvbuf, flags))) {
				os.write(rcv.data(), ret);
				if (ret < rcvbuf) {
					break; //??? check for pending data
				}
			}
			// if (ret == SOCKET_ERROR) ...

			return ret;
		}
		std::string recv(RCVMSG flags = RCVMSG::DEFAULT) const
		{
			std::ostringstream oss;

			if (INVALID_SOCKET == recv(oss, flags)) {
				throw std::runtime_error("recv failed");
			}

			return oss.str();
		}
		
		//??? no copy using stream_buf
		socket& operator>>(std::ostream& os)
		{
			int rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			std::vector<char> rcv(rcvbuf, 0);
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
		// ostream proxy
		class ostream_proxy {
			RCVMSG flags;
		public:
			ostream_proxy(const RCVMSG& flags)
				: flags(flags)
			{ }
			class recv {
				const socket<af>& s;
				ostream_proxy flags;
			public:
				recv(const socket<af>& s, ostream_proxy flags)
					: s(s), flags(flags)
				{ }
				recv(const recv&) = default;
				recv& operator=(const recv&) = default;
				recv(recv&&) = default;
				recv& operator=(recv&&) = default;
				~recv()
				{ }

				recv& operator>>(std::ostream& os)
				{
					s.recv(os, flags.flags);

					return *this;
				}
				recv& operator>>(const char* msg)
				{
					s.recv(msg, 0, flags.flags);

					return *this;
				}
				template<size_t N>
				recv& operator>>(const char(&msg)[N])
				{
					s.recv(msg, static_cast<int>(N), flags.flags);

					return *this;
				}
			};
		};
		// s >> flags(RCVMSG::XXX) >> os ...
		static ostream_proxy flags(const RCVMSG& flags)
		{
			return ostream_proxy(flags);
		}
		typename ostream_proxy::recv operator<<(const ostream_proxy& flags)
		{
			return ostream_proxy::recv(*this, flags);
		}

		int sendto(const char* buf, int len, SNDMSG flags, const ::sockaddr* to, int tolen)  const
		{
			return ::sendto(s, buf, len, static_cast<int>(flags), to, tolen);
		}
		int sendto(const sockaddr<af>& sa, const char* buf, int len, SNDMSG flags = SNDMSG::DEFAULT)  const//???MSG::CONFIRM
		{
			return sendto(buf, len, flags, &sa, sa.len());
		}

		int recvfrom(char* buf, int len, RCVMSG flags, ::sockaddr* in, int* inlen) const
		{
			return ::recvfrom(s, buf, len, static_cast<int>(flags), in, inlen);
		}
		int recvfrom(sockaddr<af>& sa, char* buf, int len, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			return recvfrom(buf, len, flags, &sa, &sa.len());
		}

	};

	// Specialize default values for constructor and member functions.
	namespace tcp {
		namespace client {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::send;
				using winsock::socket<af>::recv;
				using winsock::socket<af>::operator<<;
				using winsock::socket<af>::operator>>;

				// create and connect socket
				socket(const char* host, const char* port)
					: winsock::socket<af>(SOCK::STREAM, IPPROTO::TCP)
				{
					winsock::socket<af>::connect(host, port);
				}
			};
		}
		namespace server {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::send;
				using winsock::socket<af>::recv;
				using winsock::socket<af>::operator<<;
				using winsock::socket<af>::operator>>;

				// create socket and bind
				socket(const char* host, const char* port, AI flags = AI::PASSIVE)
					: winsock::socket<af>(SOCK::STREAM, IPPROTO::TCP)
				{ 
					::addrinfo hints = addrinfo<af>::hints(SOCK::STREAM, IPPROTO::TCP, flags);
					winsock::socket<af>::bind(addrinfo<af>(host, port, hints));
					//???accept, listen
				}
			};
		}
	}
	
	// Specialize default values for constructors
	namespace udp {
		namespace client {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::sendto;
				using winsock::socket<af>::recvfrom;
				socket()
					: winsock::socket<af>(SOCK::DGRAM, IPPROTO::UDP)
				{ }
			};
		}
		namespace server {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::sendto;
				using winsock::socket<af>::recvfrom;

				// create and bind the udp socket
				socket(const sockaddr<af>& sa)
					: winsock::socket<af>(SOCK::DGRAM, IPPROTO::UDP)
				{
					winsock::socket<af>::bind(sa);
				}
			};
		}
	}
	
	/// Define bitwise operators.
	DEFINE_ENUM_FLAG_OPERATORS(SNDMSG);
	DEFINE_ENUM_FLAG_OPERATORS(RCVMSG);
	DEFINE_ENUM_FLAG_OPERATORS(AI);
}
