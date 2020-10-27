// buffer.t.cpp - test buffers
#include <cassert>
#include <iostream>
#include <sstream>
#include "buffer.h"

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

int test_ibuffer()
{
	{
		//ibuffer b; // fails
	}
	{
		ibuffer b("abc");
		ibuffer b2(b);
		b = b2;
		auto br = b.read();
		assert(0 == strncmp("abc", &br, br.length()));
		br = b.read();
	}
	{
		ibuffer b("abc");
		ibuffer b2(b);
		b = b2;
		auto br = b.read(1);
		assert(br);
		assert(1 == br.length());
		assert(0 == strncmp("a", &br, br.length()));
		br = b.read(2);
		assert(br);
		assert(2 == br.length());
		assert(0 == strncmp("bc", &br, br.length()));
		br = b.read(1);
		assert(!br);
		assert(0 == br.length());
	}
	{
		ibuffer b(std::vector<char>(3));
		ibuffer b2(b);
		b = b2;
	}
	{
		std::vector<char> v = { 'a', 'b', 'c' };
		ibuffer b(v);
		ibuffer b2(b);
		b = b2;
		auto br = b.read(1);
		assert(br);
		assert(1 == br.length());
		assert(0 == strncmp("a", &br, br.length()));
		br = b.read(2);
		assert(br);
		assert(2 == br.length());
		assert(0 == strncmp("bc", &br, br.length()));
		br = b.read(1);
		assert(!br);
		assert(0 == br.length());
	}
	/*
	{
		std::istringstream s("abc");
		std::istringstream& rs(s);
		ibuffer b(rs);
	}
	*/

	return 0;
}


int test_ibuffer_view_ = test_ibuffer_view();
int test_ibuffer_ = test_ibuffer();
