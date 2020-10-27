// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <cstring>
//#include <concepts>
#include <string>
#include <vector>
#include <iosfwd>
#include <type_traits>

// not really winsock specific!!!
namespace winsock {

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
		void advance(int n)
		{
			if (n > buf.len) {
				n = buf.len;
			}

			buf.buf += n;
			buf.len -= n;
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

	// type of underlying buffer
	template<typename T>
	constexpr bool is_const_char = std::is_same_v<T, const char*>;
	template<typename T>
	constexpr bool is_char = std::is_same_v<T, char*>;
	template<typename T>
	constexpr bool is_string = std::is_same_v<T, std::string>;
	template<typename T>
	constexpr bool is_vector = std::is_same_v<T, std::vector<char>>;
	template<typename T>
	constexpr bool is_istream = std::is_base_of_v<std::istream, T::type>;
	template<typename T>
	constexpr bool is_ostream = std::is_base_of_v<std::ostream, T::type>;

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
			if constexpr (is_const_char<B>) {
				if (n == 0) {
					len = static_cast<int>(strlen(buf));
				}
			}
			if constexpr (is_string<B>) {
				len = static_cast<int>(buf.length());
			}
			if constexpr (is_vector<B>) {
				len = static_cast<int>(buf.size());
			}
		}

		operator bool() const
		{
			if constexpr (!is_stream<B>) {
				return off != len;
			}
			else {
				return !buf.eof();
			}
		}

		// s.send(buf(n))
		// return view on at most n characters
		ibuffer_view operator()(int n = 0)
		{
			if constexpr (is_const_char<B>) {
				const char* p = buf + off;

				// read everything
				if (n == 0 || off + n > len) {
					n = len - off;
					off = len;
				}
				else if (n > 0) {
					off += n;
				}
				//??? n < 0

				return ibuffer_view(p, n);
			}
			else if constexpr (is_string<B>) {
				const char* p = buf.data() + off;

				// read everything
				if (n == 0 || off + n > len) {
					n = len - off;
					off = len;
				}
				else if (n > 0) {
					off += n;
				}

				return ibuffer_view(p, n);
			}
			else if constexpr (is_vector<B>) {
				const char* p = buf.data() + off;

				// read everything
				if (n == 0 || off + n > len) {
					n = len - off;
					off = len;
				}
				else if (n > 0) {
					off += n;
				}

				return ibuffer_view(p, n);
			}
			else if constexpr (is_istream<B>) {
				//!!! seek offsets in std::streambuf* pbuf = buf.rdbuf();
				static char rbuf[1024];

				if (n > 0 && buf.get().good()) {
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

	// output buffer of chars to write
	template<class B = std::vector<char>>
	class obuffer {
		B buf;
		int len;
		int off = 0;   // characters read
	public:
		obuffer(size_t n = 0)
			: buf(std::vector<char>(n)), len(static_cast<int>(n))
		{ }
		obuffer(B buf, int n)
			: buf(buf), len(n)
		{ }

		operator bool() const
		{
			if constexpr (is_char<B>) {
				return off != len;
			}
			else {
				return true;
			}
		}

		char* data()
		{
			if constexpr (is_char<B>) {
				return buf;
			}
			else if constexpr (is_vector<B>) {
				return buf.data();
			}
			else {
				return nullptr;
			}
		}
		int length() const
		{
			return len;
		}

		// obuffer buf(...);
		// s.recv(buf(n));
		// s.recv(buf(n));
		// write at most 2*n chars to buf
		// return view on at most n characters
		obuffer_view operator()(int n = 0)
		{
			if constexpr (is_char<B>) {
				char* p = buf + off;

				// everything
				if (n == 0 || off + n > len) {
					n = len - off;
					off = len;
				}
				else if (n > 0) {
					off += n;
				}
				//??? n < 0

				return obuffer_view(p, n);
			}
			else if constexpr (is_vector<B>) {
				if (off + n > len) {
					buf.resize(static_cast<size_t>(off) + n);
					len = static_cast<int>(buf.size());
				}
				char* p = buf.data() + off;
				off += n;

				return obuffer_view(p, n);
			}
			else if constexpr (is_ostream<B>) {
				//!!! seek offsets in std::streambuf* pbuf = buf.rdbuf();
				return obuffer_view(buf.get().rdbuf().str().c_str(), n);
			}
			else {
				return obuffer_view{};
			}
		}
	};

}