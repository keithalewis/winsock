// winsock.t.cpp - test winsock
#include <cassert>
#include "winsock.h"

int test_socket()
{
	int i = 0;
	win::socket s;

	i = s.connect("time-a-g.nist.gov", "13");
	// time-e-wwv.nist.gov 	2610:20:6f97:97::6 	
	assert(i == 0);
	//i = s.send("GET / HTTP/1.0\r\n\r\n");
	//assert(i != 0);
	char buf[1024];
	i = s.recv(buf, 1024);
	puts(buf);

	return i;
}

int main()
{
	test_socket();

	return 0;
}