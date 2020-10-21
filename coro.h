// coro.h - coroutines
#pragma once
#include <coroutine>
#include <exception>

// Use concepts for coroutine types: task, generator, ...

struct coroutine_type {
	struct promise_type {
		int _value;
		coroutine_type get_return_object()
		{
			return { *this };
		}
		auto initial_suspend() noexcept
		{
			return std::suspend_never{};
		}
		auto final_suspend() noexcept
		{
			return std::suspend_never{};
		}
		void return_value(int val)
		{
			_value = val;
		}
		void unhandled_exception()
		{
			std::terminate();
		}
	};
	coroutine_type() = default;
	coroutine_type(const coroutine_type&) = delete;
	~coroutine_type()
	{
		_coro.destroy();
	}
private:
	coroutine_type(promise_type& p)
		: _coro(std::coroutine_handle<promise_type>::from_promise(p))
	{}
	std::coroutine_handle<promise_type> _coro;
};
