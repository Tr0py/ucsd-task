#include <string.h>
#include <assert.h>
template <class T>
class svector {
public:
	T *data;
	int _size;
	int tail;
	svector() {
		int size = 50;
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
	//TODO: T& ele
	void push_back(const T& ele) {
		int old_tail = __sync_fetch_and_add(&tail, 1);
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

