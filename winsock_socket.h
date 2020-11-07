// winsock.h - Windows socket class
// Examples from "UNIX Network Programming, Volume 1, Third Edition"
// https://github.com/unpbook/unpv13e
#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
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
#include "winsock_addr.h"
#include "winsock_buffer.h"

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

	/// <summary>
	/// Sockets parameterized by address family.
	/// </summary>
	template<AF af = AF::INET>
	class socket {
		::SOCKET s;
		// Take ownership of a raw socket.
		socket(::SOCKET s)
			: s(s)
		{ }
	public:
		socket(SOCK type, IPPROTO proto)
			: s(INVALID_SOCKET)
		{
			s = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
		}
		socket(const socket&) = delete;
		socket& operator=(const socket&) = delete;
		socket(socket&& _s) noexcept
		{
			s = std::exchange(_s.s, INVALID_SOCKET);
		}
		socket& operator=(socket&& _s) noexcept
		{
			if (s != _s.s) {
				s = std::exchange(_s.s, INVALID_SOCKET);
			}

			return *this;
		}
		~socket()
		{
			if (s != INVALID_SOCKET) {
				// may want to call shutdown first!
				::closesocket(s);
			}
		}

		/// Usable as a SOCKET
		operator ::SOCKET() const
		{
			return s;
		}

		/// <summary>
		/// Get address family, socket type, and protocol
		/// </summary>
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

		/// <summary>
		/// Retrieves the local name for a socket.
		/// </summary>
		/// This call is especially useful when a connect call has been made without doing a bind first; 
		/// this is the only way to determine the local association that has been set by the system.
		sockaddr<af> sockname() const
		{
			sockaddr<af> sa;

			ensure (0 == ::getsockname(s, &sa, &sa.len));

			return sa;
		}

		/// Address of the peer to which a socket is connected.
		sockaddr<af> peername() const
		{
			sockaddr<af> sa;

			::getpeername(s, &sa, &sa.len);

			return sa;
		}
		
		//
		// server
		//

		/// <summary>
		/// Establish the local association of the socket by assigning a local name to an unnamed socket.
		/// </summary>
		/// <param name="addr">pointer to socket address</param>
		/// <param name="len">length of bits for the address</param>
		/// <returns></returns>
		/// <remarks>
		/// The bind function is required on an unconnected socket before subsequent calls to the listen function. 
		/// It is normally used to bind to either connection-oriented (stream) or connectionless (datagram) sockets. 
		/// </remarks>
		/// See <see cref="https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind</see>"></see>
		int bind(const ::sockaddr* addr, int len) const
		{
			return ::bind(s, addr, len);
		}		
		int bind(const sockaddr<af>& sa) const
		{
			return bind(&sa, sa.len);
		}
		// Find appropriate sockadd.
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

		// Return socket on connection queue and fill in who connected.
		::SOCKET accept(::sockaddr* addr, int* len) const
		{
			return ::accept(s, addr, len);
		}
		socket accept(sockaddr<af>& sa) const
		{
			return socket(::accept(s, &sa, &sa.len));
		}
		// Ignore who connected.
		socket accept() const
		{
			return socket(::accept(s, 0, 0));
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
			return ::connect(s, &sa, sa.len);
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
			return connect(addrinfo<af>(host, port, hints()));
		}

		//
		// send
		//
		int send(const char* msg, int len = 0, SND_MSG flags = SND_MSG::DEFAULT) const
		{
			if (0 == len) {
				len = static_cast<int>(strlen(msg));
			}

			return ::send(s, msg, len, static_cast<int>(flags));
		}
		// Send data in chunks of sndbuf and return total characters sent.
		template<class T>
		int send(buffer<T>& buf, SND_MSG flags = SND_MSG::DEFAULT, int sndbuf = 0) const
		{
			int len = 0;
	
			if (0 == sndbuf) {
				sndbuf = sockopt<GET_SO::SNDBUF>(s);
			}
			
			while (const auto snd = buf(sndbuf)) {
				int ret = send(snd.buf, snd.len, flags);
				if (SOCKET_ERROR == ret) {
					return ret;
				}
				len += ret;
			}

			return len;
		}
		/*
		socket& operator<<(std::istream& msg)
		{
			if (SOCKET_ERROR == send(msg)) {
				throw std::runtime_error("winsock::socket::opertor<< failed");
			}

			return *this;
		}

		// istream proxy
		class istream_proxy {
			SND_MSG flags;
		public:
			istream_proxy(const SND_MSG& flags)
				: flags(flags)
			{ }
			class send {
				const socket<af>& s;
				istream_proxy flags;
			public:
				send(const socket<af>& s, istream_proxy flags)
					: s(s), flags(flags)
				{ }

				template<class B>
				send& operator<<(B msg)
				{
					s.send(msg, flags.flags);

					return *this;
				}
			};
		};
		// s << flags(SND_MSG::XXX) << is ...
		static istream_proxy flags(const SND_MSG& flags)
		{
			return istream_proxy(flags);
		}
		//!! static istream_proxy sndbuf(int)
		typename istream_proxy::send operator<<(const istream_proxy& flags)
		{
			return istream_proxy::send(*this, flags);
		}	
		*/
		//
		// recv
		//
		int recv(char* buf, int len, RCV_MSG flags = RCV_MSG::DEFAULT) const
		{
			return ::recv(s, buf, len, static_cast<int>(flags));
		}
		int recv(buffer<char>& buf, RCV_MSG flags = RCV_MSG::DEFAULT, int rcvbuf = 0) const
		{
			int len = 0;

			if (0 == rcvbuf) {
				rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			}

			while (auto rcv = buf(rcvbuf)) {
				int ret = recv(rcv.buf, rcv.len, flags);

				if (SOCKET_ERROR == ret) {
					DWORD err;
					err = GetLastError();
					return ret;
				}
				len += ret;
				if (ret < rcv.len) {
					break;
				}
			}

			return len;
  		}
		/*
		template<class B>
		socket& operator>>(obuffer<B>& buf)
		{
			int ret = recv(buf);
			if (SOCKET_ERROR == ret) {
				throw std::runtime_error("winsock::socket::opertor<< failed");
			}

			return *this;
		}
		socket& operator>>(std::ostream& os)
		{
			obuffer buf(std::ref(os));

			return operator>>(buf);
		}
		

		// ostream proxy
		class ostream_proxy {
			RCV_MSG flags;
		public:
			ostream_proxy(const RCV_MSG& flags)
				: flags(flags)
			{ }
			class recv {
				const socket<af>& s;
				ostream_proxy flags;
			public:
				recv(const socket<af>& s, ostream_proxy flags)
					: s(s), flags(flags)
				{ }
				template<class B>
				recv& operator>>(B& buf)
				{
					s.recv(obuffer(buf), flags.flags);

					return *this;
				}
			};
		};
		// s >> flags(RCV_MSG::XXX) >> os ...
		static ostream_proxy flags(const RCV_MSG& flags)
		{
			return ostream_proxy(flags);
		}
		typename ostream_proxy::recv operator<<(const ostream_proxy& flags)
		{
			return ostream_proxy::recv(*this, flags);
		}
		*/
		int sendto(const char* buf, int len, SND_MSG flags, const ::sockaddr* to, int tolen)  const
		{
			return ::sendto(s, buf, len, static_cast<int>(flags), to, tolen);
		}
		int sendto(const sockaddr<af>& to, const char* buf, int len, SND_MSG flags = SND_MSG::DEFAULT)  const//???MSG::CONFIRM
		{
			return sendto(buf, len, flags, &to, to.len);
		}

		int recvfrom(char* buf, int len, RCV_MSG flags, ::sockaddr* from, int* fromlen) const
		{
			return ::recvfrom(s, buf, len, static_cast<int>(flags), from, fromlen);
		}
		int recvfrom(sockaddr<af>& from, char* buf, int len, RCV_MSG flags = RCV_MSG::DEFAULT) const
		{
			return recvfrom(buf, len, flags, &from, &from.len);
		}

	};
	static_assert(sizeof(winsock::socket<>) == sizeof(::SOCKET));

	// Specialize default values for constructor and member functions.
	namespace tcp {
		namespace client {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::socket;
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::sockname;
				using winsock::socket<af>::peername;
				using winsock::socket<af>::connect;
				using winsock::socket<af>::send;
				using winsock::socket<af>::recv;
				//using winsock::socket<af>::operator<<;
				//using winsock::socket<af>::operator>>;

				// create and connect socket
				socket(const char* host, const char* port)
					: winsock::socket<af>(SOCK::STREAM, IPPROTO::TCP)
				{
					connect(host, port);
				}
				socket(const sockaddr<af>& sa)
					: winsock::socket<af>(SOCK::STREAM, IPPROTO::TCP)
				{
					connect(sa);
				}
			};
		}
		namespace server {
			template<AF af = AF::INET>
			class socket : private winsock::socket<af> {
			public:
				using winsock::socket<af>::socket;
				using winsock::socket<af>::operator ::SOCKET;
				using winsock::socket<af>::sockname;
				using winsock::socket<af>::peername;
				using winsock::socket<af>::bind;
				using winsock::socket<af>::listen;
				using winsock::socket<af>::accept;
				using winsock::socket<af>::send;
				using winsock::socket<af>::recv;
				//using winsock::socket<af>::operator<<;
				//using winsock::socket<af>::operator>>;

				// create socket and bind
				socket(const char* host, const char* port, AI flags = AI::PASSIVE)
					: winsock::socket<af>(SOCK::STREAM, IPPROTO::TCP)
				{ 
					sockopt<SET_SO::REUSEADDR>(*this, true);
					::addrinfo hints = addrinfo<af>::hints(SOCK::STREAM, IPPROTO::TCP, flags);
					bind(addrinfo<af>(host, port, hints));
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
				using winsock::socket<af>::socket;
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
	DEFINE_ENUM_FLAG_OPERATORS(SND_MSG);
	DEFINE_ENUM_FLAG_OPERATORS(RCV_MSG);
	DEFINE_ENUM_FLAG_OPERATORS(AI);
}
