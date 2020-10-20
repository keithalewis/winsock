// winsock.t.cpp - test winsock
#include <cassert>
#include <cstdio>
#include "winsock.h"

int test_socket()
{
	int i = 0;
	{
		win::socket s;

		i = s.connect("time-a-g.nist.gov", "13");
		// time-e-wwv.nist.gov 	2610:20:6f97:97::6 	
		assert(i == 0);
		//i = s.send("GET / HTTP/1.0\r\n\r\n");
		//assert(i != 0);
		char buf[1024];
		buf[s.recv(buf, 1024)] = 0;
		puts(buf);
	}
	{
		win::socket s;

		assert (0 == s.connect("time-a-g.nist.gov", "13"));
		assert(i == 0);
		s >> std::cout;
	}
	{
		win::socket s;
		assert(0 == s.connect("google.com", "https"));
		s << "GET / HTTP/1.0\r\n\r\n";
		s >> std::cout;
	}

	return i;
}

int main()
{
	test_socket();

	return 0;
}