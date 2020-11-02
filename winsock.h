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
#include "winsock_addr.h"

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
			return socket(::accept(s, &sa, &sa.len()));
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

		//
		// send
		//
		int send(const char* msg, int len, SNDMSG flags = SNDMSG::DEFAULT)
		{
			return ::send(s, msg, len, static_cast<int>(flags));
		}
		// Send data in chunks of sndbuf and return total characters sent.
		int send(const ibuffer& buf, SNDMSG flags = SNDMSG::DEFAULT, int sndbuf = 0) const
		{
			int len = 0;
	
			if (0 == sndbuf) {
				sndbuf = sockopt<GET_SO::SNDBUF>(s);
			}
			
			while (const ibuffer snd = buf(sndbuf)) {
				int ret = s.send(&snd, snd.len, static_cast<int>(flags));
				if (SOCKET_ERROR == ret) {
					return ret;
				}
				len += ret;
			}

			return len;
		}
		/*
		template<class B>
		socket& operator<<(B msg)
		{
			if (SOCKET_ERROR == send(msg)) {
				throw std::runtime_error("winsock::socket::opertor<< failed");
			}

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

				template<class B>
				send& operator<<(B msg)
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
		*/
		//
		// recv
		//
		int recv(char* buf, int len, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			return ::recv(s, buf, len, static_cast<int>(flags));
		}
		template<class B>
		int recv(obuffer& buf, RCVMSG flags = RCVMSG::DEFAULT, int rcvbuf = 0) const
		{
			int len = 0;

			if (0 == rcvbuf) {
				rcvbuf = sockopt<GET_SO::RCVBUF>(s);
			}

			while (obuffer rcv = buf(rcvbuf)) {
				int ret = s.recv(&rcv, rcv.len, flags);

				if (SOCKET_ERROR == ret) {
					return ret;
				}
				if (0 == ret) {
					break;
				}

				len += ret;
				if (ret < rcv.len) {
					buf.length(len);

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
				template<class B>
				recv& operator>>(B& buf)
				{
					s.recv(obuffer(buf), flags.flags);

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
		*/
		int sendto(const char* buf, int len, SNDMSG flags, const ::sockaddr* to, int tolen)  const
		{
			return ::sendto(s, buf, len, static_cast<int>(flags), to, tolen);
		}
		int sendto(const sockaddr<af>& to, const char* buf, int len, SNDMSG flags = SNDMSG::DEFAULT)  const//???MSG::CONFIRM
		{
			return sendto(buf, len, flags, &to, to.len());
		}

		int recvfrom(char* buf, int len, RCVMSG flags, ::sockaddr* from, int* fromlen) const
		{
			return ::recvfrom(s, buf, len, static_cast<int>(flags), from, fromlen);
		}
		int recvfrom(sockaddr<af>& from, char* buf, int len, RCVMSG flags = RCVMSG::DEFAULT) const
		{
			return recvfrom(buf, len, flags, &from, &from.len());
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
				//using winsock::socket<af>::operator<<;
				//using winsock::socket<af>::operator>>;

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
				//using winsock::socket<af>::operator<<;
				//using winsock::socket<af>::operator>>;

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
