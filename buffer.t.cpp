// buffer.t.cpp - test buffers
#include <cassert>
#include <iostream>
#include <sstream>
#include "winsock_buffer.h"

using namespace winsock;

int test_ibuffer_view()
{
	{
		ibuffer_view b;
		assert(0 == b.length());
		assert(nullptr == &b);

		ibuffer_view b2(b);
		assert(b2.length() == b.length());
		assert(&b2 == &b);

		b = b2;
		assert(b2.length() == b.length());
		assert(&b2 == &b);
	}
	{
		ibuffer_view b("abc", 3);
		assert(3 == b.length());
		assert('a' == *&b);
		assert(0 == strncmp("abc", &b, b.length()));
	}

	return 0;
}

int test_obuffer_view()
{
	{
		obuffer_view b;
		assert(0 == b.length());
		assert(nullptr == &b);
		assert(!b);

		obuffer_view b2(b);
		assert(b2.length() == b.length());
		assert(&b2 == &b);

		b = b2;
		assert(b2.length() == b.length());
		assert(&b2 == &b);
	}
	{
		const char* abc = "abc";
		char buf[3];
		obuffer_view b(buf, 3);
		assert(3 == b.length());
		assert(buf == &b);
		memcpy(&b, abc, 3);
		assert(0 == strncmp(abc, &b, b.length()));
	}

	return 0;
}

template<class B>
int test_ibuffer(ibuffer<B>& buf, const char* s, int n)
{
	ibuffer<B> b2(buf);
	buf = b2;

	if (n == 0) {
		auto br = buf();
		assert(0 == strncmp(s, &br, br.length()));
	}
	else {
		auto br = buf(n);
		assert(br);

		assert(n == br.length());
		assert(0 == strncmp(s, &br, n));
		br = buf();
		assert(0 == strncmp(s + n, &br, br.length()));
	}

	return 0;
}

int test_ibuffer()
{
	{
		//ibuffer b; // fails
	}
	const char abc[] = "abc";
	{
		ibuffer b(abc);
		test_ibuffer(b, abc, 0);
	}
	{
		ibuffer b(abc);
		test_ibuffer(b, abc, 1);
	}
	{
		ibuffer b(abc);
		test_ibuffer(b, abc, 2);
	}
	{
		ibuffer b(abc);
		test_ibuffer(b, abc, 3);
	}
	{
		std::string s(abc);
		ibuffer b(s);
		test_ibuffer(b, abc, 1);
	}
	{
		std::vector<char> v(abc, abc + 3);
		ibuffer b(v);
		test_ibuffer(b, abc, 2);
	}
	{
		std::istringstream s(abc);
		ibuffer b(std::ref(s));
		test_ibuffer(b, abc, 3);
	}
	
	return 0;
}

int test_obuffer()
{
	{
		char buf[3];
		obuffer b(buf, 3);
		auto wb = b(1);
		assert(1 == wb.length());
		*&wb = 'a';
		wb = b(2);
		assert(!b);
		assert(2 == wb.length());
		memcpy(&wb, "bc", 2);
		assert(0 == strncmp(buf, "abc", 3));
	}
	{
		obuffer b;
		obuffer b2(b);
		b = b2;
		auto wb = b(3);
		assert(b);
		assert(3 == wb.length());
		memcpy(&wb, "abc", wb.length());
		assert(0 == strncmp(b.data(), "abc", 3));
		wb = b(2);
		assert(b);
		memcpy(&wb, "de", wb.length());
		assert(0 == strncmp(b.data(), "abcde", 5));
	}
	{
		obuffer b(std::ref(std::cout));
	}
	{
		//!!!obuffer b(std::cout);
	}

	return 0;
}

int test_ibuffer_view_ = test_ibuffer_view();
int test_obuffer_view_ = test_obuffer_view();
int test_ibuffer_ = test_ibuffer();
int test_obuffer_ = test_obuffer();
