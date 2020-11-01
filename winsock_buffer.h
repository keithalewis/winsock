// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <cstdio>
#include <cstring>
//#include <concepts>
#include <string>
#include <vector>
#include <iosfwd>
#include <type_traits>

// not really winsock specific!!!
namespace winsock {

	// view on buffer of Ts
	template<typename T>
	struct buffer_view {
		T* buf;
		int len;

		buffer_view(T* buf, size_t len)
			: buf(buf), len(static_cast<int>(len))
		{ }
		explicit buffer_view(std::string& buf)
			: buffer_view(buf.data(), buf.length())
		{ }
		explicit buffer_view(std::vector<char>& buf)
			: buffer_view(buf.data(), buf.size())
		{ }

		// buffer<B> b; while (buf = b(n)) { send(buf.buf, buf.len); }
		operator bool() const
		{
			return len != 0;
		}
	};
	using ibuffer_view = buffer_view<const char>;
	using obuffer_view = buffer_view<char>;

	// type of underlying buffer
	template<typename B>
	constexpr bool is_char = std::is_same_v<B, char*>;
	template<typename B>
	constexpr bool is_string = std::is_same_v<B, std::string>;
	template<typename B>
	constexpr bool is_vector = std::is_same_v<B, std::vector<char>>;
	template<typename B>
	constexpr bool is_file = std::is_same_v<B, FILE*>;
	template<typename B>
	constexpr bool is_iostream = std::is_base_of_v<std::iostream, B::type>;
	template<typename B>
	constexpr bool is_istream = std::is_base_of_v<std::istream, B::type>;
	template<typename B>
	constexpr bool is_ostream = std::is_base_of_v<std::ostream, B::type>;

	// buffers of chars
	template<class B> requires (is_char<B> || is_string<B> || is_vector<B> || is_iostream<B>)
	class buffer {
		B buf;
		int len;
		int off; // characters read
	public:
		buffer(B buf, int n = 0)
			: buf(buf), len(n), off(0)
		{
			if constexpr (is_char<B>) {
				if (n == 0) {
					len = static_cast<int>(strlen(buf));
				}
			}
			else if constexpr (is_string<B>) {
				len = static_cast<int>(buf.length());
			}
			else if constexpr (is_vector<B>) {
				len = static_cast<int>(buf.size());
			}
			else {
				len = -1; // B is file or iostream
			}
		}

		// buffer not empty
		operator bool() const
		{
			if constexpr (is_char<B> || is_string<B> || is_vector<B>) {
				return off != len;
			}
			else if constexpr (is_file<B>) {
				return !::feof(buf);
			}
			else if constexpr (is_iostream<B>) {
				return !buf.eof();
			}
			else {
				return false;
			}
		}
		

		// s.send(buf(n))
		// return view on at most n characters
		template<class T>
		buffer_view<T> operator()(size_t n = 0)
		{
			if constexpr (is_char<B> || is_string<B> ||  is_vector<B>) {
				T* p = &buf[0] + off;

				// read everything
				if (n == 0 || off + n > len) {
					n = len - off;
				}
				else {
					off += static_cast<int>(n);
				}

				return buffer_view<T>(p, n);
			}
			/*
			if constexpr (is_istream<B>) {
				//!!! seek offsets in std::streambuf* pbuf = buf.rdbuf();
				static char rbuf[1024];
				auto& get = buf.get();

				if (n > 0 && get) {
					get.read(rbuf, n);
					n = static_cast<int>(get.gcount());
				}

				return ibuffer_view(rbuf, n);
			}
			*/
			else {
				return buffer_view{};
			}
		}
	};

	// !!! for now
	template<class B>
	using ibuffer = buffer<B>;
	template<class B>
	using obuffer = buffer<B>;
	/*
	// output buffer of chars to write
	template<class B = std::vector<char>>
	class obuffer {
		B buf;
		int len;
		int off = 0;   // characters read
	public:
		explicit obuffer(size_t n = 0)
			: buf(std::vector<char>(n)), len(static_cast<int>(n))
		{ }
		obuffer(B buf, int n = 0)
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
			else if constexpr (is_vector<B> || is_string<B>) {
				return buf.data();
			}
			// ostringstream return str().data()
			else {
				return nullptr;
			}
		}
		int length() const
		{
			return len;
		}
		void resize(int n)
		{
			if constexpr (is_vector<B> || is_string<B>) {
				buf.resize(n);
				len = n;
			}
		}

		// obuffer buf(...);
		// s.recv(buf(n));
		// s.recv(buf(n));
		// write at most 2*n chars to buf
		// return view on at most n characters
		buffer_view<char> operator()(int n = 0)
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
			else if constexpr (is_string<B>) {
				if (off + n > len) {
					buf.resize(static_cast<size_t>(off) + n);
					len = static_cast<int>(buf.length());
				}
				char* p = buf.data() + off;
				off += n;

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
				return obuffer_view{}; // (buf.get().rdbuf().str().c_str(), n);
			}
			else {
				return obuffer_view{};
			}
		}
	};
	*/

}