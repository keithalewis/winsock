# winsock

A header only affordances for Windows sockets.

Sockets and associated structures are parameterized by their _address family_
so the C++ type system ensures compatibility. Enumerations are
defined in `enum class`es to ensure argument correctness and provide
intellisence assistance.

## `winsock::socket<AF>`

The `socket` class provides type safe member functions for basic socket functions:
`bind`, `accept`, `listen`, `connect`, `send`, `recv`, `sendto`, and `recvfrom`.
The `winsock::socket` class requires the address family (`AF`) template parameter.
The constructor has two required arguments for the socket type (`SOCK`) and protocol (`IPPROTO`).
The class also implements `operator ::SOCKET()` so a `socket` can be used in any
function having a Windows `SOCKET` argument.

The `send` and `recv` functions can use `std::istream` and `std::ostream` for output
and input to and from sockets. There are corresponding member functions
`operator<<` and `operator>>` and a `flags` member function that return a correponding
stream proxy class in order to do local IO manipulation.

For example, if `s` is a `socket` then `s << flags(SNDMSG::OOB) << "Hello";` calls
`send(s, "Hello", 5, MSG_OOB)`. The flags stay in effect only for the duration of
the statement, which is a feature.

## `sockaddr<AF>`

## `addrinfo<AF>`

## `winsock::tcp`

This namespace contains classes for TCP socke streams.

## `winsock::tcp::client::socket<AF>`

This class provides appropriate defaults for TCP clients. The constructor takes host and port
strings that are used by `getaddrinfo` and calls `connect`. There is also a constructor
that takes a socket address.

## `winsock::tcp::server::socket<AF::INET>`

This class provides appropriate defaults for TCP servers. The constructor takes host and port
strings that are used by `getaddrinfo` and calls `bind`. There is also a constructor
that takes a socket address. It does not call `accept` or `listen`.

## `winsock::udp`

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
