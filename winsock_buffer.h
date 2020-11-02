// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <cstring>
#include <Windows.h>

// not really winsock specific!!!
namespace winsock {

	// RAII Windows unique HANDLE
	// using handle = std::unique_ptr<HANDLE,decltype(&::CloseHandle)>; drags in too much interface
	class handle {
		HANDLE h;
	public:
		handle(HANDLE h = INVALID_HANDLE_VALUE)
			: h(h)
		{ }
		handle(const handle&) = delete;
		handle& operator=(const handle&) = delete;
		handle(handle&& _h) noexcept
		{
			h = _h.h;
			_h.h = INVALID_HANDLE_VALUE;
		}
		handle& operator=(handle&& _h) noexcept
		{
			if (h != _h.h) {
				h = _h;
				_h.h = INVALID_HANDLE_VALUE;
			}
			
			return *this;
		}
		~handle()
		{
			if (INVALID_HANDLE_VALUE != h) {
				::CloseHandle(h);
			}
		}
		operator HANDLE() const
		{
			return h;
		}
	};

	// view on buffer of Ts in chunks of at most N (where N should be system page size)
	template<typename T, size_t N = 0x1000>
	struct buffer {
		T* buf;
		int len;
		int off;

		buffer(T* buf, size_t len)
			: buf(buf), len(static_cast<int>(len)), off(0)
		{ }

		operator bool() const
		{
			return off < len;
		}

		T& operator[](size_t i)
		{
			if (i >= len) {
				throw std::runtime_error("buffer::operator[]: index out of range");
			}

			return buf[i];
		}
		const T& operator[](size_t i) const
		{
			if (i >= len) {
				throw std::runtime_error("buffer::operator[] const: index out of range");
			}

			return buf[i];
		}

		// set offset
		void offset(int o = 0)
		{
			off = o;
		}

		// buffer<B> b; while (buf = b(n)) { send(buf.buf, buf.len); }
		// Return new buffer view of [off, off + n) chars and increment offset
		buffer operator()(size_t n = 0)
		{
			T* p = buf + off;

			if (n > N) {
				n = N;
			}

			// all availble data
			if (n == 0 || off + n > len) {
				n = static_cast<size_t>(len) - off;
				off = len;
			}
			else {
				off += static_cast<int>(n);
			}

			return buffer(p, n);
		}

	};

	struct ibuffer : public buffer<const char>
	{
		ibuffer(const char* buf, int len = 0)
			: buffer<const char>(buf, len ? len : static_cast<int>(strlen(buf)))
		{ }
		// ibuffer b("abc") works
		template<size_t N>
		ibuffer(const char (&buf)[N])
			: ibuffer(buf, N)
		{ }
	};
	struct obuffer : public buffer<char>
	{
		obuffer(char* buf, int len = 0)
			: buffer<char>(buf, len ? len : static_cast<int>(strlen(buf)))
		{ }
		void length(int l)
		{
			len = l;
		}
	};

	// buffer backed by anonymous file
	class iobuffer : public buffer<char>
	{
		handle h;
	public:
		// default to 1MB
		iobuffer(int len = 1 << 20)
			: buffer<char>(nullptr, len)
			, h(CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, len, NULL))
		{
			if (h) {
				buf = (char*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, len);
			}
		}
		iobuffer(const iobuffer&) = delete;
		iobuffer& operator=(const iobuffer&) = delete;
		~iobuffer()
		{
			if (buf) {
				UnmapViewOfFile(buf);
			}
		}
	};

	// ifbuffer, ofbuffer, iofbuffer for files
}