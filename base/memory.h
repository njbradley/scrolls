#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#include "common.h"

template <typename T>
struct RefCounted : public T {
	using T::T;
	RefCounted(const T& other): T(other) {}
	int refcount = 0;
	
	void incref() {
		refcount ++;
	}
	void decref() {
		refcount --;
		if (refcount == 0) delete this;
	}
};

template <typename T>
struct RefCounter {
	RefCounted<T>* pointer = nullptr;
	
	RefCounter() {}
	RefCounter(RefCounted<T>* ptr): pointer(ptr) {
		if (pointer != nullptr) pointer->incref();
	}
	RefCounter(const RefCounter<T>& other): RefCounter(other.pointer) {}
	RefCounter(RefCounter<T>&& other) {
		std::swap(pointer, other.pointer);
	}
	~RefCounter() {
		if (pointer != nullptr) pointer->decref();
	}
	
	RefCounter<T>& operator=(RefCounter<T> other) {
		std::swap(pointer, other.pointer);
		return *this;
	}
	RefCounted<T>* operator->() { return pointer; }
	const RefCounted<T>* operator->() const { return pointer; }
	RefCounted<T>& operator*() { return *pointer; }
	const RefCounted<T>& operator*() const { return *pointer; }
	operator RefCounted<T>*() const { return pointer; }
};

#endif
