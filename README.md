# winsock

A header only set of utilities for Windows sockets.

Sockets and associated structures by parameterized by _address family_
to have the C++ type system to ensure compatibility. Enumerations are
defined in `enum class`es to ensure argument correctness and provide
intellisence assistence.

## `winsock::socket<AF>`

The `socket` class provides type safe member functions for basic socket functions:
`bind`, `accept`, `listen`, `connect`, `send`, `recv`, `sendto`, and `recvfrom`.
The constructor requires the address family to be either `AF::INET` or `AF::INET6`
and has two required arguments for the socket type (`SOCK`) and protocol (`IPPROTO`).
The class also implements `operator ::SOCKET()` so a `socket` can be used in any
function having a Windows `SOCKET` argument.

The `send` and `recv` functions can use `std::istream` and `std::ostream` for output
and input to and from sockets. There are corresponding member functions
`operator<<` and `operator>>` and a `flags` member function that return a correponding
stream proxy class in order to do local IO manipulation.

For example, if `s` is a `socket` then `s << flags(SNDMSG::OOB) << "Hello";` calls
`send(s, "Hello", 5, MSG_OOB)`. The flags stay in effect only for the duration of
the statement, which is a feature.

## `winsock::tcp::client<AF>`

This class provides appropriate defaults for TCP clients. 

```
tcp::server::socket<AF::INET> s(host, port);
```

## UDP server
``
udp::server::socket<> s(IADDR::ANY, 1234); // create and bind
while (0 < s.recv(sa, buf, len, MSG::WAITALL) {
	s.sendMSG::CONFIRM) << ios;
}
```

## UDP client
```
int ret;
sockaddr<> sa(IADDR::ANY, 1234);
udp::client::socket<> s(sa);
s.send(buf, len, flags); // -> chars sent
s.recv(buf, len, flags); // -> chars received
```