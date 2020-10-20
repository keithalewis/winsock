// winsock.t.cpp - test winsock
#include <cassert>
#include <cstdio>
#include "winsock.h"

using namespace winsock;

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

int test_socket()
{
	int i = 0;
	{
		winsock::socket s;

		// time-e-wwv.nist.gov 	2610:20:6f97:97::6 	
		i = s.connect("time-a-g.nist.gov", "13");
		assert(i == 0);
		char buf[1024];
		buf[s.recv(buf, 1024)] = 0;
		puts(buf);
	}
	{
		winsock::socket s;

		assert (0 == s.connect("time-a-g.nist.gov", "13"));
		assert(i == 0);
		s >> std::cout;
	}
	/*
	{
		winsock::socket s;
		assert(0 == s.connect("google.com", "http"));
		s.send("GET / HTTP/1.1\r\nHost: google.com\r\n\r\n");
		//s.send("Host: google.com\r\n\r\n");
		char buf[64];
		i = s.recv(buf, 64);
		puts(buf);
	}
	*/
	{
		winsock::socket s;
		assert(0 == s.connect("www.google.com", "http"));
		s << "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
		s >> std::cout;
	}

	return i;
}

int test_hints()
{
	winsock::socket s;
	auto hint = s.hints();
	//assert((int)AF::UNIX == hint.ai_family);
	//auto family = getsockopt<SO::TYPE>(s);
	auto type = getsockopt<SO::TYPE>(s);

	return 0;
}


int main()
{
	test_hints();
	test_socket();

	return 0;
}