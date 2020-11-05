// winsock.t.cpp - test winsock
#include <cassert>
#include <cstdio>
#include <thread>
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

template<AF af = AF::INET>
inline void tcp_server_echo(const char* host = "localhost", const char* port = "8888")
{
	tcp::server::socket<af> s(host, port, AI::PASSIVE);
	s.listen();
	iobuffer buf;
	while (true) {
		winsock::socket<af> t = s.accept();
		t.recv(buf);
		t.send(buf);
	}
}

template<AF af>
int test_tcp_server_echo()
{
	std::thread echo(tcp_server_echo<af>, "localhost", "6789");
	tcp::client::socket<af> s("localhost", "6789");
	char buf[1024];
	buffer<char> iob(buf, 1024);

	echo.join();

	return 0;
}
int test_tcp_server_echo_ = test_tcp_server_echo<AF::INET>();

/*
int test_hints()
{
	winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
	auto hint = s.hints();
	auto type = sockopt<GET_SO::TYPE>(s);
	assert(type == hint.ai_socktype);

	return 0;
}
*/

#if 0

int test_socket()
{
	int i = 0;
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
	// sudo mknod -m 777 fifo p
	// cat fifo | netcat -l -k localhost 8000 > fifo
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);
		i = s.connect("localhost", "8000");
		i = s.send("hi", 2);
		char buf[1024];
		i = s.recv(buf, 1023);
		if (i != SOCKET_ERROR) {
			buf[i] = 0;
		}
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
		//assert(0 < n && n <= 1023);
		buf[n] = 0;
	}
	{
		winsock::socket<> s(SOCK::STREAM, IPPROTO::TCP);

		// time-e-wwv.nist.gov 	2610:20:6f97:97::6 	
		i = s.connect("time-a-g.nist.gov", "13");
		assert(i == 0);
		char buf[1024] = { 0 };
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
		s << winsock::socket<>::flags(SND_MSG::DEFAULT) << "GET / HTTP/1.1" << "\r\n\r\n";
		//obuffer buf(std::ref(std::cout));
		s >> std::cout;
	}
	{
		winsock::tcp::client::socket<> s("www.google.com", "http");
		//ibuffer buf("GET / HTTP/1.1\r\n\r\n");
		s.send("GET / HTTP/1.1\r\n\r\n");
		obuffer buf;
		s.recv(buf);
		std::cout.write(buf.data(), buf.length());
	}

	return i;
}

int test_udp_socket()
{
	winsock::sockaddr<> sa(INADDR::ANY, 2345);
	udp::server::socket<> srv(sa);
	srv.sendto(sa, "hi", 2);
	winsock::sockaddr<> srvsa;
	int ret;
	ret = getpeername(srv, &srvsa, &srvsa.len);

	udp::client::socket<> cli;
	char buf[3];
	int i;
	i = cli.recvfrom(sa, buf, 3);
	//!!!assert(i == 2);

	return 0;
}

#endif // 0

int main()
{
	//test_hints();
	/*
	test_socket();
	test_udp_socket();
	*/

	return 0;
}