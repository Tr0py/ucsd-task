template <class T>
class svector {
public:
	T *data;
	int size;
	int tail;
	svector(int size) {
		data = new T[size * sizeof(T)];
		this->size = size;
		tail = 0;
	}
	~svector() {
		delete[] data;
	};
	void push_back(T& ele) {
		int old_tail = __sync_fetch_and_add(&tail, 1);
		data[old_tail] = ele;
	}
	T& operator[](int& idx) {
		return data[idx];
	}
	int cur_size() {
		return tail;
	}
};

