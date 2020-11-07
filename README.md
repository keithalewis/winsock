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
by necessity deals with the low level fiddling involved in shipping bits
between two computers connected by a network.

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
iobuffer msg("message"); // memory mapped anonymous buffer
s.send(msg);
s.recv(msg);
```
The buffer now contains the response the server gave to the message
unless something went haywire.
It is possible the server was not
listening or it recieved the message and did not send a response.
It is also possible the server isn't even running so your socket will
never connect. Well written socket code takes all possibilities into account.

A server program to echo messages sent using TCP back to the client looks like this:
```C++
// bind to port 2345 on this machine with AI_PASSIVE flag
tcp::server::socket<> s("localhost", "2345", AI::PASSIVE);
iobuffer buf; 

::listen(s, SOMAXCONN); // call listen with system recommended backlog

while (true) {
	socket<> c = s.accept(); // socket to any client that connected
	c.recv(buf); // read what client sent
	c.send(buf); // write it back to them
} // socket<> destructor calls ::closesocket(c)
```
Server sockets call [`::bind`](https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind).
The `tcp::server::socket<>` constructor creates a TCP stream socket and calls
`bind` with the specified flag.
The call to [`::listen`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen)
uses the member function `operator ::SOCKET()` to return
the underlying `::SOCKET`. This enables a `socket<>` to be used in any socket API function. 
It also means I can write less code. Code not written never has bugs.
The call to [`accept`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept)
creates a new socket whenever a client connects on the bound port.
The socket needs to be closed after using it and
the C++ class for `socket<>` does that for you when it goes out of scope.

This implementation has an infinite loop. A better implementation would provide a means of
breaking out of the loop and call [`::shutdown`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-shutdown)
on the client facing socket so it knows the server intends to close the socket.
That way the client can infer the server didn't simply crash.  

## Buffer

Buffers use `buffer_view`s to return chunks of data appropriate for sending and
receiving on sockets. It is a struct with members `buf` pointing to character data 
and `len` which is the `int` length of the data required by socket API functions.

The `buffer` class provides backing for the characters sent and received over sockets.
The constructor takes a character pointer and the length of the data.
The private member `int off` keeps track how much of the buffer has been used.
The member function `operator()(size_t n)` returns a `buffer_view` having
at most `n` characters starting from the offset and then increments the offset. 
If `n` is `0` (or omitted) then all available buffer capacity is used.
When the end of data is reached it may return a view with `len` less than `n`.
The offset is then set to 0 for reuse. You can do this
manually by calling the member function `reset()` if necessary.

Buffers are designed to make this unnecessary when used in a loop:
```
while (buffer_view chunk = buf(n)) {
	// send or receive chunk of size at most n
}
// buf offset is now 0 and ready for reuse
```
Each chunk has length `n` until the penultimate chunk that gives a view on remaining
data. The last chunk has size `0` and `buffer_view::operator bool() const` returns
`false` to terminate the loop.

Full disclosure: buffers also have a `size_t` template parameter limiting the
chunk size. The default value is `0x1000` which is the typical page
size for memory on most operating systems. 
You can change that if you think you know what you are doing.

The derived class `ibuffer` is a `buffer<const char>` with an
additional constructor
from a `const (&char)[N]` so `ibuffer b("abc")` can be used. 
Input buffers are used for sending characters.

The derived class `obuffer` is a `buffer<char>` and
is used for receiving characters into a preallocated buffer
given its length.

The derived class `cbuffer` allocates characters for a given length.
Be sure to construct the class and give it a name before using it.
The `send` and `recv` functions take references to buffers so
they need to be l-values. You are responsible for managing the
lifetime of a `cbuffer`. There are specializations for `icbuffer`
and `ocbuffer` using `const char` and `char` template parameters.

A buffer would not be a buffer if it did not do buffering.
When receiving data it is not known how much data will
eventually arrive. Instead of allocating memory and then reallocating
when more than the expected amount of data arrives, we use _memory mapped files_
and offload buffering to the operating system.

By default, the derived class `iobuffer` is a
`buffer<char>` backed by an anonymous memory mapped file. It
can be used to both send and receive characters. The constructor
sets `len` to the maximum allowed size, which is 1GB by default.
Don't worry, the operating system does not allocate that
memory until you write to it. If you stick to `buffer::operator()(int n)`
to return chunks of buffer views the OS will take care of the buffering
and an exception will be thrown if you step out of bounds.
You can alse construct them from handles returned by `CreateFile`
if you want to (eventually) do disk I/O.

The buffer classes are completely independent of sockets but probably only useful when using those.

## `sockaddr<AF>`

To use a socket you need to know its _address_.  
The `sockaddr<AF>` class is a _value type_ with sufficient room to hold addresses. 
It has a constructor that takes an _IPv4 or IPv6 internet network address_ `const char*` string 
and an `unsigned short` port number then calls
[inet_pton`](https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_pton) 
to parse the string into requisite bits and convert the port into appropriate byte order
to send over a network.
The `operator&()` member function and `int len;` member supply 
standard socket API functions access to the low level address bits they need.
You should never need to touch `len` since that is determined by the address family.

If you know your party (IP address) and extension (port) there is no need to
involve network calls to specify a socket address.

## `addrinfo<AF>`

The function [`getaddrinfo`](https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo)
is used to get information about addresses if you don't know the IP address.

It returns a list of potential socket addresses given a host, port, and hints.
The host name string and hints are sent over the network to get a list of IP address.
The port string can be the digits of the port or a 'well-known' port name such as `"https"` or `"ping"`,
where well-known means your local machine is set up to convert that into the actual unsigned short
port number. The hints indicate the address family, socket type, protocol, and optional
flags telling the host how the socket is intended to be used.

The socket member functions `socket<>::bind` and `socket<>::connect` supply the
appropriate hints and traverse proffered addresses until finding one that the
host deems acceptible. The `addrinfo_iter` class is used to walk through
the list of addresses returned by `getaddrinfo`, but you don't need to know that.'

## `winsock::socket<AF>`

The `socket<>` class provides type safe member functions for basic socket functions:
`bind`, `accept`, `listen`, `connect`, `send`, `recv`, `sendto`, and `recvfrom`.
The `winsock::socket<AF>` class indicates the address family (`AF`) by the template parameter.
The default is `AF::INET` for sockets using IPv4. The address family `AF::INET6`
creates a socket that uses IPv6. Other address families can be used, but they are rare.
The constructor has two required arguments for the socket type (`SOCK`) and protocol (`IPPROTO`).
The class implements `operator ::SOCKET()` so a `socket` can be used in any
function having a Windows `SOCKET` argument.

The `send(const ibuffer&, SND_MSG)` and `recv(obuffer&, RCV_MSG)` member functions use the buffer classes 
to make character data available and set flags for the 
[`::send`](https://docs.microsoft.com/en-us/windows/win32/api/Winsock2/nf-winsock2-send) and 
[`::recv`](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-recv) socket API function.

The member functions `operator<<(std::istream&)` and `operator>>(std::ostream&)` can also be used to send or
receive stream data.
The static member function `socket<>::flags` can be used to pass flags to streams.

For example, if `s` is a `socket<>` then `s << flags(SND_MSG::OOB) << "Hello";` calls
`::send(s, "Hello", 5, MSG_OOB)`. The flags stay in effect only for the duration of
the statement, which is a feature.

## `winsock::tcp`

This namespace contains classes for TCP stream sockets. 
The TCP protocol provide guarantees about delivery. 
If `send` or `recv` return `0` then the transmission was successful.
There is a price to pay for this. Both ends of a TCP connection
need to have a little chat in order to set up a socket and
sent or received data use acknowledgements to guarantee delivery.

## `winsock::tcp::client::socket<AF>`

This class provides appropriate defaults for TCP clients. The constructor takes host and port
strings that are used by `getaddrinfo` and calls `connect`. There is also a constructor
that takes a socket address.

## `winsock::tcp::server::socket<AF::INET>`

This class provides appropriate defaults for TCP servers. The constructor takes host and port
strings that are used by `getaddrinfo` and calls `bind`. There is also a private constructor
that takes a socket address. It does not call `accept` or `listen`.

## `winsock::udp`

This namespace contains classes for UDP datagram sockets.
The UDP protocol provides no guarantees about delivery.
The `send` and `recv` functions fire-and-forget. They give
no indication whatsoever about whether delivery was successful.
Since there is no need to go through the trouble of negotiating
a network connection or waiting for acknowledgements they are much faster
than TCP sockets.

```
udp::server::socket<> s(IADDR::ANY, 1234); // create and bind
while (0 < s.recv(sa, buf, len, MSG::WAITALL) {
	s.send<< flags(MSG::CONFIRM) << ios;
}
```

### UDP client

```
int ret;
sockaddr<> sa(IADDR::ANY, 1234);
udp::client::socket<> s(sa);
s.send(buf, len, flags); // -> chars sent
s.recv(buf, len, flags); // -> chars received
```
