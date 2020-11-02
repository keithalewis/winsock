// buffer.h - buffer using char array, vector, iostream
#pragma once
#include <Windows.h>

// not really winsock specific!!!
namespace winsock {

	// RAII Windows unique HANDLE
	// using handle = std::unique_ptr<HANDLE,decltype(&::CloseHandle)>; ???
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

	static const SYSTEM_INFO& SystemInfo()
	{
		static bool first = true;
		static SYSTEM_INFO si;

		if (first) {
			GetSystemInfo(&si);
			first = false;
		}

		return si;
	}

	// Round down to system allocation granularity
	inline DWORD round(DWORD n)
	{
		DWORD ag = SystemInfo().dwAllocationGranularity;

		return ag * (n / ag);
	}

	class file_view {
		handle h;
		void* v;
	public:
		/*
		file_view(HANDLE h,
			DWORD  flags,
			DWORD  hi,
			DWORD  lo,
			SIZE_T n)
			: h(MapViewOfFile(file, flags, hi, lo, n))
		{ }
		*/
		file_view(DWORD n)
			: h(CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, n, NULL))
			, v(MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, SystemInfo().dwPageSize))
		{ }
		file_view(const file_view&) = delete;
		file_view& operator=(const file_view&) = delete;
		~file_view()
		{
			if (INVALID_HANDLE_VALUE != h) {
				UnmapViewOfFile(h);
			}
		}
		char* operator&()
		{
			return (char*)v;
		}
	};

	// view on buffer of Ts
	template<typename T>
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

		// buffer<B> b; while (buf = b(n)) { send(buf.buf, buf.len); }
		// Return [off, off + n) chars
		buffer operator()(size_t n = 0)
		{
			T* p = buf + off;

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

	using ibuffer = buffer<const char>;
	using obuffer = buffer<char>;
}