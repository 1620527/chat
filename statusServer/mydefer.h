#pragma once

template<class Func>
struct Sdefer {
	Sdefer(Func func) :f(func) {}
	~Sdefer() {
		f();
	}
	Func f;
};

template<class Func>
Sdefer<Func> MakeDefer(Func f) {
	return Sdefer(f);
}



#include <functional>

class Defer {
public:
	Defer(std::function<void()> func) :_func(func){}
	~Defer() {
		_func();
	}
	std::function<void()> _func;
};