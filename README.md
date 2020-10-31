# winsock

Header only affordances for Windows sockets.

File descriptors are used for reading and writing files on a disk. 
A `pipe` is a file descriptor for reading and writing between two executables running on the same machine.
Sockets are pipes where the executables can be running on different machines connected by a network.
Leslie Lamport defined a distributed system as "one in which the failure of a computer you didn't even 
know existed can render your own computer unusable." This library makes it easy
to get to the subtle and difficult to reason about problems involved in distributed computing.
It provides training wheels for the pesky minutia involved with creating sockets
and setting them up for reading and writing. The socket API is quite ancient and
by necessity deals with the low level machinery involved in shipping bits
between any two computers connected to a network.

The design philosophy is to allow use of all standard socket API functions
and only provide simple wrappers for common patterns of using sockets
that are type safe and leverage RAII best practices. 
This library will help you avoid making dumb mistakes. 
You can make smart mistakes in the rest of your code later.

Sockets and their associated structures are parameterized by _address family_
and enumerations are defined using `enum class`
so the C++ type system enforces compatible calls to the socket API. 
A buffer class is provided to isolate the mechanics of
providing characters to the send and recv socket calls used to write
and read data using a socket.

A client program to send and receive a message using the TCP protocol to server 
`host` listening on `port` looks like this:
```C++
tcp::client::socket s(host, port);  // connect to host on port
buffer<std::string> msg("message"); // string backed buffer
s.send(msg);
s.recv(msg);
```
The string buffer now contains the response from the server to the message
unless something went haywire. It is possible the server was not
listening for messages or it recieved the message and did not send a response.

A server program to echo messages sent using TCP back to the client looks like this:
```C++
// bind to port 2345 on this machine with AI_PASSIVE flag
tcp::server::socket<> s("localhost", "2345", AI::PASSIVE);
buffer<std::vector<char>> buf; // use vector char buffer 

::listen(s, SOMAXCONN); // call listen with system suggested backlog

while (true) {
	socket<> c = s.accept(); // socket to any client that connected
	c.recv(buf); // read what client sent
	c.send(buf); // write it back to them
} // socket<> destructor calls ::closesocket(c)
```
Server sockets call [`bind`](https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind).
The `tcp::server::socket<>` constructor creates a TCP stream socket and calls
`bind` with the specified flag.
A C++ `std::string` can have embedded `0` characters but `std::vector<char>` does not give those special treatment.
The call to [`listen`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen)
uses the member function `operator ::SOCKET()` to return
the underlying `::SOCKET`. This enables a `socket<>` to be used in any socket API function. 
The call to [`accept`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept)
creates a new socket when a client connects on the bound port. That needs to be closed after using it.
The C++ class for `socket<>` does that for you when it goes out of scope.

## Buffer

The `buffer` class provides backing for the characters sent and received over sockets.
The member function `operator()(int n)` returns a `buffer_view` class having 
member functions `operator&()` and `length()` to provide access to at most `n` characters
to be transmitted by sockets. If `n` is omitted then all available buffer capacity is used.
Buffers can be backed by fixed length character arrays, strings, vectors of characters,
files, or IO streams.
There are specializations `ibuffer` and `obuffer` that indicate the buffer will only
be used for sending and receiving socket data, respectively. This might seem to
be the reverse of the functionality they provide but `socket::operator<<(ibuffer&)`
and `socket::operator>>(obuffer&)` member functions behave as expected.

The buffer classes are completely independent of sockets but probably only useful when using those.

## `sockaddr<AF>`

To use a socket you need to know its _address_.  
The `sockaddr<AF>` class is a _value type_ with sufficient room to hold addresses. 
It has a constructor that takes an _IPv4 or IPv6 internet network address_ string and an unsigned short port number then calls
[inet_pton`](https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_pton) 
to parse the string into requisite bits and convert the port into appropriate byte order
to send over a network.
The `operator&()` and `len()` member functions supply 
standard socket API functions access to the low level address bits they need.

If you know your party (IP address) and extension (port) there is no need to
involve network calls to specify an address.

## `addrinfo<AF>`

The function [`getaddrinfo`](https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo)
is used to get information about addresses if you don't know the IP address.

It returns a list of potential socket addresses given a host, port, and hints.
The host string name is sent over the network to get a list of IP address.
The port string can be the digits of the port or a 'well-known' port name such as `"https"` or `"telnet"`,
where well-known means your local machine is set up to convert that into the actual unsigned short
port number. The hints indicate the address family, socket type, protocol, and optional
flags specifying how the socket is intended to be used.

The socket member functions `socket<>::bind` and `socket<>::connect` supply the
appropriate hints and traverse proffered addresses until finding one that the
remote machine deems acceptible. The `addrinfo_iter` class is used to walk through
the potential addresses returned by `getaddrinfo`.

## `winsock::socket<AF>`

The `socket` class provides type safe member functions for basic socket functions:
`bind`, `accept`, `listen`, `connect`, `send`, `recv`, `sendto`, and `recvfrom`.
The `winsock::socket<AF>` class has an address family (`AF`) template parameter
that defaults to `AF::INET` for sockets using IPv4. The address family `AF::INET6`
creates a socket that uses IPv6. Other address families can be used, but they are rare.
The constructor has two required arguments for the socket type (`SOCK`) and protocol (`IPPROTO`).
The class implements `operator ::SOCKET()` so a `socket` can be used in any
function having a Windows `SOCKET` argument.

The `send(const ibuffer&, SNDMSG)` and `recv(obuffer&, RCVMSG)` member functions use the buffer classes 
to make character data available and set flags for the 
[`send`](https://docs.microsoft.com/en-us/windows/win32/api/Winsock2/nf-winsock2-send) and 
[`recv`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-recv) socket API function.

The member functions `operator<<(std::istream&)` and `operator>>(std::ostream&)` can also be used to send or
receive data.
The `socket<>::flags` static member function can be used to pass flags to these functions.

For example, if `s` is a `socket<>` then `s << flags(SNDMSG::OOB) << "Hello";` calls
`::send(s, "Hello", 5, MSG_OOB)`. Unlike `std::iomanip` functions, the flags stay in effect only for the duration of
the statement, which is a feature.

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
