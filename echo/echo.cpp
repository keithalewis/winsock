// echo.cpp - echo server
#include "../winsock.h"

using namespace winsock;

template<AF af, class B>
void tcp_server_echo(const char* host = "localhost", const char* port = "8888")
{

	tcp::server::socket<af> s(host, port, AI::PASSIVE);
	s.listen();
	winsock::socket<af> t = s.accept();
	while (0 < t.recv(buf)) {
		t.send(buf);
	}
}

int main()
{
	return 0;
}