// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <cstring>
#include <concepts>
#include <string>
#include <vector>
#include <iosfwd>
#include <type_traits>

// buffer view
template<class T>
struct buffer_view {
	T* buf;
	int len;
	operator bool() const
	{
		return len != 0;
	}
};

class ibuffer_view {
	buffer_view<const char> buf;
public:
	ibuffer_view(const char* buf = nullptr, int len = 0)
		: buf{buf,len}
	{ }

	operator bool() const
	{
		return buf.len != 0;
	}

	const char* operator&() const
	{
		return buf.buf;
	}
	int length() const
	{
		return buf.len;
	}
};

class obuffer_view {
	buffer_view<char> buf;
public:
	obuffer_view(char* buf = nullptr, int len = 0)
		: buf{ buf, len }
	{ }

	operator bool() const
	{
		return buf.len != 0;
	}

	char* operator&()
	{
		return buf.buf;
	}
	int& length()
	{
		return buf.len;
	}
};

/*
template<class T>
using is_const_char = std::is_same_v<typename T, const char*>;
template<class T>
using is_char = std::is_same_v<typename T, char*>;
*/

// input buffer of chars to read
template<class B>
class ibuffer {
	B buf;
	int len;
	int off = 0;   // characters read
public:
	ibuffer(B buf, int n = 0)
		: buf(buf), len(n)
	{ 
		if constexpr (std::is_same_v<B, const char*>) {
			if (n == 0) {
				len = static_cast<int>(strlen(buf));
			}
		}
		if constexpr (std::is_same_v<B, std::vector<char>>) {
			len = static_cast<int>(buf.size());
		}
	}

	operator bool() const
	{
		// eof, off < buf.size(), 
		return true; // return len != off; // both 0 ???
	}

	// s.send(buf.read(n))
	// return view on at most n characters
	ibuffer_view read(int n = 0)
	{
		if constexpr (std::is_same_v<B, const char*>) {
			const char* p = buf + off;

			if (n > 0) {
				if (off + n > len) {
					n = len - off;
					off = len;
				}
				else {
					off += n;
				}
			}

			return ibuffer_view(p, n);
		}
		else if constexpr (std::is_same_v<B, std::vector<char>>) {
			const char* p = buf.data() + off;

			if (n > 0) {
				if (off + n > len) {
					n = len - off;
					off = len;
				}
				else {
					off += n;
				}
			}

			return ibuffer_view(p, n);
		}
		else if constexpr (std::is_base_of_v<std::istream, B::type>) {
			static char rbuf[1024];

			if (n > 0 && !buf.get().eof()) {
				buf.get().read(rbuf, n);
				n = static_cast<int>(buf.get().gcount());
			}

			return ibuffer_view(rbuf, n);
		}
		else {
			return ibuffer_view{};
		}
	}
};

