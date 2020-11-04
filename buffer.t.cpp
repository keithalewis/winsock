// buffer.t.cpp - test buffers
#include <cassert>
#include <iostream>
#include <sstream>
#include "winsock_buffer.h"

using namespace winsock;

DWORD scratch()
{
	DWORD ret = 0;
	HANDLE h = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 10, NULL);
	LPVOID v;
	if (h) {
		v = MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000);
		if (v) {
			char* p;
			p = (char*)v;
			p[0] = 'a';
			assert(0 == strncmp(p, "a", 1));
			p[9] = 'b';
			p[10] = 'c';
			assert('c' == p[10]);
			/*
			for (int i = 1000; i; i += 1000) {
				p[i] = 0;
			}
			*/
		}
	}

	return ret;
}
DWORD scratch_ = scratch();

int test_buffer()
{
	{
		char buf[3] = { 'a', 'b', 'c' };
		ibuffer ib(buf, 3);
		ibuffer ib2{ ib };
		ib = ib2;
		assert(ib.len == 3);
		assert(0 == strncmp(buf, ib.buf, ib.len));

		char c = 'a';
		while (const auto b = ib(1)) {
			assert(1 == b.len);
			assert(c++ == *b.buf);
		}
		assert('d' == c);

		ib.reset();
		const auto b = ib(100);
		assert(3 == b.len);
		assert(buf == b.buf);
	}
	{
		ibuffer ib("abc");
		ibuffer ib2{ ib };
		ib = ib2;
		assert(ib.len == 3);
		assert(0 == strncmp("abc", ib.buf, ib.len));
	}
	{
		char buf[3] = { 'a', 'b', 'c' };
		obuffer ob(buf, 3);
		obuffer ob2{ ob };
		ob = ob2;
		assert(ob.len == 3);
		assert(0 == strncmp(buf, ob.buf, ob.len));
		memcpy_s(ob.buf, ob.len, "def", 3);
		assert(ob.len == 3);
		assert(0 == strncmp("def", ob.buf, ob.len));
	}
	{
		iobuffer b;
		memcpy_s(b.buf, 3, "ghi", 3);
		assert(0 == strncmp("ghi", b.buf, b.len));
		auto ob = b(4);
		memcpy_s(ob.buf, ob.len, "jklm", 4);
		assert(0 == strncmp("jklm", b.buf, 4));
	}

	return 0;
}

int test_buffer_ = test_buffer();