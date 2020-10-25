# winsock

A header only affordances for Windows sockets.

Sockets and associated structures are parameterized by _address family_
so the C++ type system ensures compatibility. Enumerations are
defined in `enum class`es to ensure argument correctness and provide
intellisence assistence.

## `winsock::socket<AF>`

The `socket` class provides type safe member functions for basic socket functions:
`bind`, `accept`, `listen`, `connect`, `send`, `recv`, `sendto`, and `recvfrom`.
The `winsock::socket` class requires an address family (`AF`) template parameter.
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

This is a _value type_ with sufficient room to hold addresses. It has a constructor
that takes a _dotted IP address_ string and an unsigned short port number. It calls
`inet_pton` to fill in the address. The `::sockeraddr_in* operator&() const`, 
`int len() const` and `int& len()` member functions are used in various socket related functions.

## `addrinfo<AF>`

This can be used to look up approriate `sockaddr_in` data given a host and port string.
When `bind` or `connect` is called it will traverse `ai_next` pointers until it finds
a successful bind or connect address.

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
