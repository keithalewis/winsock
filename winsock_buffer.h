// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <cstring>
#include <stdexcept>
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

	// view on buffer of Ts 
	template<typename T>
	struct buffer_view {
		T* buf;
		int len;
		operator bool() const
		{
			return len != 0;
		}
	};

	// buffer of T*s in chunks of at most N (where N should be system page size)
	template<typename T, size_t N = 0x1000>
	class buffer : public buffer_view<T> {
		int off;
	public:
		using buffer_view<T>::buf;
		using buffer_view<T>::len;

		buffer()
			: buffer_view<T>{nullptr, 0}
		{ }
		buffer(T* buf, size_t len)
			: buffer_view<T>{ buf, static_cast<int>(len) }, off(0)
		{ }
		template<size_t N>
		buffer(T (&buf)[N])
			: buffer_view<T>{ buf, static_cast<int>(N) }, off(0)
		{ }

		//!!! probably a bad idea
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

		// reset offset
		void reset()
		{
			off = 0;
		}

		// buffer<T> b; while (buf = b(n)) { send(buf.buf, buf.len); }
		// Return new buffer view of [off, off + n) chars and increment offset
		buffer_view<T> operator()(size_t n = 0)
		{
			if (len == off) {
				reset();

				return buffer_view<T>{};
			}

			if (n > N) {
				n = N;
			}

			if (n == 0 || off + n > len) {
				// all available data
				n = static_cast<size_t>(len) - off;
			}
				
			T* p = buf + off;
			off += static_cast<int>(n);

			return buffer_view{ p, static_cast<int>(n) };
		}

	};
	
	using ibuffer = buffer<const char>;
	using obuffer = buffer<char>;

	// file backed buffer
	template<class T = char>
	class iobuffer : public buffer<T>
	{
		handle m;
	public:
		using buffer<T>::buf;
		using buffer<T>::len;

		iobuffer(HANDLE h, DWORD flags, DWORD hi, DWORD lo, LPCTSTR name = nullptr)
			: buffer<char>(nullptr, lo)
			, m(CreateFileMapping(h, NULL, flags, hi, lo, name))
		{
			if (m) {
				buf = (char*)MapViewOfFile(m, FILE_MAP_ALL_ACCESS, 0, 0, len);
			}
		}
		// anonymous temporary mapped file
		iobuffer(DWORD len = 1<<20)
			: iobuffer(INVALID_HANDLE_VALUE, PAGE_READWRITE, 0, len)
		{ }
		iobuffer(const iobuffer&) = delete;
		iobuffer& operator=(const iobuffer&) = delete;
		// movable???
		~iobuffer()
		{
			if (buf) {
				UnmapViewOfFile(buf);
			}
		}
	};

}