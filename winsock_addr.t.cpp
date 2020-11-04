// winsock_addr.t.cpp - Test winsock address classes
#include <cassert>
#include "winsock_addr.h"

using namespace winsock;

template<AF af>
int test_sockaddr()
{
	winsock::sockaddr<af> sa(inaddr<af>::any, 12345);
	winsock::sockaddr<af> sa2{ sa };
	assert(sa2 == sa);
	sa = sa2;
	assert(sa == sa2);
	assert(!(sa != sa2));
	//assert(sa <= sa2);
	//assert(!sa < sa2);


	// by hand
	typename inaddr<af>::sockaddr_type sin;
	//::sockaddr_in sin;
	memset(&sin, 0, sizeof(inaddr<af>::sockaddr_type));
	inaddr<af>::family(sin) = static_cast<int>(af);
	inaddr<af>::addr(sin) = inaddr<af>::any;
	inaddr<af>::port(sin) = htons(12345);

	assert(0 == memcmp(&sin, &sa, sa.len));

	return 0;
}
int test_sockaddr_ = test_sockaddr<AF::INET>();
int test_sockaddr6_ = test_sockaddr<AF::INET6>();

int test_addrinfo()
{
	{
		::addrinfo ai = winsock::addrinfo<>::hints(SOCK::STREAM, winsock::IPPROTO::TCP, AI::DEFAULT);
		assert(ai.ai_flags == 0);
		assert(ai.ai_family == AF_INET);
		assert(ai.ai_socktype == SOCK_STREAM);
		assert(ai.ai_protocol == IPPROTO_TCP);
	}

	return 0;
}
int test_addrinfo_ = test_addrinfo();

