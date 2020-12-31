#include <iostream>
#include<vector>
#include<map>
#include<iostream>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<fstream>
#include <sstream>
#include <omp.h>
using namespace std;
template <class T>
class svector {
	T *data;
	int size;
	int tail;
public:
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

int test_vector() 
{
	vector<int> vec;
	vec.resize(200000);
	cout << vec.size()<<endl;
#pragma omp parallel for
	for (int i=0; i < 10000; i++) {
		vec.push_back(i);
	}

	cout << "vector:";
	for (int i=0; i < 10000; i++) {
		cout << vec[i];
	}
	cout << endl;
}

int test_svector()
{
	svector<int> vec(20000);
	cout << vec.cur_size()<<endl;
#pragma omp parallel for
	for (int i=0; i < 10000; i++) {
		vec.push_back(i);
	}

	cout << "vector:";
	for (int i=0; i < 10000; i++) {
		cout << vec[i] << " ";
	}
	cout << endl;
}

int main()
{

	test_svector();
	
	return 0;
}
