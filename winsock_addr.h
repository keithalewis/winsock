// winsock_addr.h - socket addresses
#pragma once
#include <array>
#include <compare>
#include <stdexcept>
#include "winsock_enum.h"

namespace winsock {

	/// Value object from dotted IPv4 or IPv6 address string
	
	/// <summary>
	/// A value type holding a contiguous set of bits that specify a transport address.
	/// </summary>
	/// <remarks>
	/// A socket address consists of the address family of the protocol used by the socket,
	/// the host address, and a port number.
	/// </remarks>
	template<AF af = AF::INET>
	class sockaddr {
		std::array<char, sizeof(inaddr<af>::sockaddr_type)> sa;
	public:
		int len; // For use in socket API calls.

		using address_family = AF;

		sockaddr()
			: sa{}, len(static_cast<int>(sa.size()))
		{
			family(af);
		}
		sockaddr(typename const inaddr<af>::addr_type& _addr, unsigned short _port)
			: sockaddr()
		{
			addr(_addr);
			port(htons(_port));
		}
		sockaddr(const char* host, unsigned short _port)
			: sockaddr()
		{
			typename inaddr<af>::addr_type _addr;
			int ret = inet_pton(static_cast<int>(af), host, &_addr);
			if (ret != 1) {
				//!!! use std::system_error and WSAGetLastError()
				throw std::runtime_error("inet_pton failed");
			}
			addr(_addr);
			port(htons(_port));
		}
		sockaddr(const sockaddr&) = default;
		sockaddr& operator=(const sockaddr&) = default;
		sockaddr(sockaddr&&) = default;
		sockaddr& operator=(sockaddr&&) = default;
		~sockaddr()
		{ }

		bool operator==(const sockaddr& _sa) const
		{
			if (len != _sa.len) {
				return false;
			}

			for (size_t i = 0; i < len; ++i) {
				if (sa[i] != _sa.sa[i]) {
					return false;
				}
			}

			return true;
		}

		/// <summary>
		/// Cast data to raw pointer to be used with socket API functions.
		/// </summary>
		::sockaddr* operator&()
		{
			return (::sockaddr*)sa.data();
		}
		const ::sockaddr* operator&() const
		{
			return (const ::sockaddr*)sa.data();
		}

		/// <summary>
		/// Cast data to appropriate address family.
		/// </summary>
		typename inaddr<af>::sockaddr_type& in()
		{
			return *reinterpret_cast<typename inaddr<af>::sockaddr_type*>(sa.data());
		}

		/// <summary>
		/// Address components
		/// </summary>
		AF family() const
		{
			return static_cast<AF>(inaddr<af>::family(in()));
		}
		void family(AF _af)
		{
			inaddr<af>::family(in()) = static_cast<ADDRESS_FAMILY>(_af);
		}
		typename const inaddr<af>::addr_type& addr() const
		{
			return inaddr<af>::addr(in());
		}
		void addr(typename const inaddr<af>::addr_type& _addr)
		{
			inaddr<af>::addr(in()) = _addr;
		}
		unsigned short port() const
		{
			return inaddr<af>::port(in());
		}
		void port(unsigned short _port)
		{
			inaddr<af>::port(in()) = _port;
		}

	};

	/// <summary>
	/// The addrinfo class is used by the getaddrinfo function to hold host address information.
	/// </summary>
	template<AF af = AF::INET>
	class addrinfo {
		::addrinfo* pai;
	public:
		/// Forward iterator over addrinfo pointers
		class addrinfo_iter;

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
		//!!! Add functions to get info about recommended packet size for address.
		
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

	};


}