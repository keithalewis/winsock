// winsock_addr.h - socket addresses
#pragma once
#include <compare>
#include "winsock_enum.h"

namespace winsock {

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


}