// echo.cpp - echo server
#include "../winsock.h"

using namespace winsock;

template<AF af = AF::INET>
void tcp_server_echo(const char* host = "localhost", const char* port = "8888")
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

int main()
{
	tcp_server_echo();

	return 0;
}