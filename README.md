# winsock

A header only affordances for Windows sockets.

File descriptors are used for reading and writing files on a disk. 
A `pipe` is a file descriptor for reading and writing between two executables running on the same machine.
Sockets are pipes where the executables can be running on different machines connected by a network.
Leslie Laport defined a distributed system as "one in which the failure of a computer you didn't even 
know existed can render your own computer unusable." This library makes it easy
to get to the difficult to reason about and solve problems involved in networked computing.
It provides training wheels for the pesky minutia involved with creating sockets
and setting them up for reading and writing. The socket API is quite ancient and
by necessity deals with the low level machinery involved with shipping bits
between any two computers connected to a network.

The design philosophy is to allow use of all standard socket related functions
and only provide simple wrappers for common patterns using modern type safe
and RAII best practices. This library will help you avoid making dumb mistakes. 
You can make smart mistakes in the rest of your code later.

Sockets and associated structures are parameterized by _address family_
so the C++ type system enforces compatible calls to the socket API. 
Enumerations are defined in `enum class`es to ensure argument correctness and provide
intellisence assistence. A buffer class is provided to isolate the mechanics of
providing characters to the send and recv socket calls.

A client program to send and receive a message using TCP to server 
`host` listening on `port` looks like this:
```C++
tcp::client::socket s(host, port);  // connect to host on port
buffer<std::string> msg("message"); // string backed buffer
s.send(msg);
s.recv(msg);
```
The string buffer now contains the response from the server to the message.

A server program to echo messages back to the client looks like this:
```
 // bind to port 2345 on this machine with AI_PASSIVE flag
tcp::server::socket<> s("localhost", "2345", AI::PASSIVE);
buffer<std::vector<char>> buf; // use vector char buffer 

::listen(s, SOMAXCONN); // call listen(2) with system suggested backlog

while (true) {
	socket<> c = s.accept(); // socket to any client that connected
	c.send(buf); // read what client sent
	c.recv(buf); // write it back to them
} // socket<> destructor calls ::closesocket(c)
```
Server sockets need to call `bind(2)` with appropriate flags. 
C++ `std::string` can have embedded `0` characters, but `std::vector` does not give those special treatment.
The call to `accept(2)` creates a new socket that needs to be closed after using it.
The C++ class for `socket<>` does that for you when it goes out of scope.

## Buffer

The `buffer` class provides backing for the characters sent over sockets.
It is completely independent of sockets but probably only useful when using those.

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
