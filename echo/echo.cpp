// echo.cpp - echo server
#include "../winsock.h"

using namespace winsock;

template<AF af>
void tcp_server_echo(const char* port = "8888")
{

	tcp::server::socket<af> s("localhost", "8888", AI::PASSIVE);
	s.listen();
	s.accept();
}

int main()
{
	return 0;
}