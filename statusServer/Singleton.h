#pragma once
#include "myprint.h"

template<class T>
class Singleton{
protected:
	Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
public:
	static T& GetInstance() {
		static T instance;
		return instance;
	}
	void printAddress() {
		print(&(GetInstance()));
	}
	~Singleton() {
		print("this is singleton destruct");
	}
};