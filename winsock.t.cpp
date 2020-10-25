// winsock.t.cpp - test winsock
#include <cassert>
#include <cstdio>
#include <future>
#include "winsock.h"

using namespace winsock;
using winsock::socket;
using winsock::IPPROTO;

struct bl {
	const char* buf;
	int len;
};
struct sr {
	bl send;
	bl recv;
};
const sr data[] = {
	{
		{"foo", 3},
		{"bar", 3}
	}
};

int test_sockaddr()
{
	winsock::sockaddr<> sa(INADDR::ANY, 12345);

	// by hand
	::sockaddr_in sin;
	memset(&sin, 0, sizeof(::sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(12345);

	assert(0 == memcmp(&sin, &sa, sa.len()));

	return 0;
}

int test_addrinfo()
{
	::addrinfo ai = winsock::addrinfo<>::hints(SOCK::STREAM, IPPROTO::TCP, AI::DEFAULT);
	assert(ai.ai_flags == 0);
	assert(ai.ai_family == AF_INET);
	assert(ai.ai_socktype == SOCK_STREAM);
	assert(ai.ai_protocol == IPPROTO_TCP);

	return 0;
}

int test_socket()
{
	int i = 0;
	static_assert(sizeof(winsock::socket<>) == sizeof(::SOCKET));
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		assert(sockopt<GET_SO::TYPE>(s) == SOCK_STREAM);
		auto pi = sockopt<GET_SO::PROTOCOL_INFO>(s);
		assert(pi.iAddressFamily == AF_INET);
		assert(pi.iSocketType == SOCK_STREAM);
		assert(pi.iProtocol == IPPROTO_TCP);
		
		assert(0 == sockopt<SET_SO::SNDBUF>(s, 10));
		assert(sockopt<GET_SO::SNDBUF>(s) == 10);
	}
	// https://tools.ietf.org/html/rfc862
	{
		winsock::sockaddr<> sa("127.0.0.1", IPPORT::ECHO);
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		s.connect(sa);
		s.send("hi", 2);
		char buf[3];
		s.recv(buf, 2);
	}
	// https://www.ietf.org/rfc/rfc5905.txt
	{
		winsock::sockaddr<> sa("132.163.96.2", IPPORT::TIMESERVER);
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		s.connect(sa);
	}
	{
		// https://ipecho.net/plain 
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		s.connect("ipecho.net", "https");
		s.send("GET /plain HTTP/1.1\r\nHost: ipecho.net\r\n\r\n");
		//s.send("GET /plain HTTP/1.1\r\n\r\n");
		char buf[1024];
		int n = s.recv(buf, 1024);
		assert(0 < n && n <= 1023);
		buf[n] = 0;
	}
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);

		// time-e-wwv.nist.gov 	2610:20:6f97:97::6 	
		i = s.connect("time-a-g.nist.gov", "13");
		assert(i == 0);
		char buf[1024];
		buf[s.recv(buf, 1024)] = 0;
		puts(buf);
	}
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);

		assert (0 == s.connect("time-a-g.nist.gov", "13"));
		assert(i == 0);
		std::string rcv = s.recv();
		assert(rcv.size() != 0);
		rcv.push_back(0);
		puts(rcv.data());
		//s >> std::cout;
	}
	/*
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		assert(0 == s.connect("google.com", "http"));
		s.send("GET / HTTP/1.1\r\nHost: google.com\r\n\r\n");
		//s.send("Host: google.com\r\n\r\n");
		char buf[64];
		i = s.recv(buf, 64);
		puts(buf);
	}
	*/
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		assert(0 == s.connect("www.google.com", "http"));
		winsock::sockaddr<> sa;
		sa = s.peername();
		s.send("GET / HTTP/1.1\r\n\r\n");
		s >> std::cout;
	}
	{
		winsock::tcp::client::socket<> s("www.google.com", "http");
		s << winsock::socket<>::flags(SNDMSG::DEFAULT) << "GET / HTTP/1.1\r\n\r\n";
		s >> std::cout;
	}

	return i;
}

int test_hints()
{
	winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
	auto hint = s.hints();
	auto type = sockopt<GET_SO::TYPE>(s);
	assert(type == hint.ai_socktype);

	return 0;
}

int test_udp_socket()
{
	winsock::sockaddr<> sa(INADDR::ANY, 2345);
	udp::server::socket<> srv(sa);
	srv.sendto(sa, "hi", 2);
	winsock::sockaddr<> srvsa;
	int ret;
	ret = getpeername(srv, &srvsa, &srvsa.len());

	udp::client::socket<> cli;
	char buf[3];
	int i;
	i = cli.recvfrom(sa, buf, 3);
	//!!!assert(i == 2);

	return 0;
}

int main()
{
	test_sockaddr();
	test_addrinfo();
	test_hints();
	test_socket();
	test_udp_socket();

	return 0;
}