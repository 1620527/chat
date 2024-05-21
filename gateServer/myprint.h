#pragma once


#include <iostream>

template<class T>
void print(T t) {
	std::cout << t << std::endl;
}

template<class T, class ...ARGS>
void print(T t, ARGS ...args) {
	std::cout << t << " ";
	print(args...);
}