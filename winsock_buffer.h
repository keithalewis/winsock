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
	class buffer_view {
		T* buf;
		int len;
	public:
		buffer_view(const char* buf = nullptr, int len = 0)
			: buf{ buf, len }
		{ }

		operator bool() const
		{
			return len != 0;
		}

		char* operator&()
		{
			return buf;
		}
		const char* operator&() const
		{
			return buf;
		}
		int& length()
		{
			return len;
		}
		int length() const
		{
			return len;
		}
		// move start of view forward n chars
		void advance(int n)
		{
			// ensure n >= 0 ???
			if (n > len) {
				buf = nullptr;
				len = 0;
			}

			buf += n;
			len -= n;
		}
	};

	// type of underlying buffer
	constexpr bool is_char = std::is_same_v<T, char*>;
	template<typename T>
	constexpr bool is_string = std::is_same_v<T, std::string>;
	template<typename T>
	constexpr bool is_vector = std::is_same_v<T, std::vector<char>>;
	template<typename T>
	constexpr bool is_file = std::is_same_v<T, FILE*>;
	template<typename T>
	constexpr bool is_iostream = std::is_base_of_v<std::iostream, T::type>;
	template<typename T>
	constexpr bool is_istream = std::is_base_of_v<std::istream, T::type>;
	template<typename T>
	constexpr bool is_ostream = std::is_base_of_v<std::ostream, T::type>;
	template<typename T>
	constexpr bool is_file = std::is_same_v<T, FILE*>;

	// buffers of chars
	template<class B> requires (is_char<B> || is_string<B> || is_vector<B> || is_iostream<B>)
	class buffer {
		B buf;
		int len; // const ???
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
			if constexpr (is_string<B>) {
				len = static_cast<int>(buf.length());
			}
			if constexpr (is_vector<B>) {
				len = static_cast<int>(buf.size());
			}
			else {
				len = -1; // B is iostream
			}
		}

		// buffer not empty
		operator bool() const
		{
			if constexpr (is_char<B> || is_string<B> || is_vector<B>) {
				return off != len;
			}

			return false;
		}

		// s.send(buf(n))
		// return view on at most n characters
		const buffer_view operator()(int n = 0)
		{
			if constexpr (is_char<B>) {
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

				return buffer_view(p, n);
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
				auto& get = buf.get();

				if (n > 0 && get) {
					get.read(rbuf, n);
					n = static_cast<int>(get.gcount());
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

}