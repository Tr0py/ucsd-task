#include <string.h>
#include <assert.h>
#include "config.h"
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
template <class T>
class svector {
public:
	T *data;
	volatile int _size;
	int tail;
	svector() {
		//TODO: profile size 90% 50%
		int size = SVEC_INIT_SIZE;
		data = new T[size * sizeof(T)];
		this->_size = size;
		tail = 0;
	}
	svector(int size) {
		data = new T[size * sizeof(T)];
		this->_size = size;
		tail = 0;
	}
	svector(const svector<T>& cp) {
		// Don't do this! It's slow!
		assert(0);
		_size = cp._size;
		data = new T[_size * sizeof(T)];
		memcpy(data, cp.data, _size * sizeof(T));
		tail = cp.tail;
	}

	~svector() {
		delete[] data;
	};
	void push_back(const T& ele) {
		int old_tail = __sync_fetch_and_add(&tail, 1);
		if (unlikely(old_tail == _size)) {
			T* old_data = data;
			data = new T[_size+5 * sizeof(T)];
			memcpy(data, old_data, sizeof(T) * _size);
			asm volatile("": : :"memory");
			_size += 5;
			delete[] old_data;
		}
		/* waiting for alloc */
		while (unlikely(old_tail >= _size)) ;
		data[old_tail] = ele;
		//data[tail++] = ele;
	}
	T& operator[](int idx) const {
		return data[idx];
	}
	int size()  const {
		return tail;
	}
	/*
	T* begin() const {
		return data;
	}
	T* end() const {
		return &(data[tail+1]);
	}
	*/
};

