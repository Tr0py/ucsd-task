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
#include "../svec.h"
using namespace std;
#define VEC_SIZE 200000000
int verify_vec[VEC_SIZE];

int test_vector() 
{
	vector<int> vec;
	vec.resize(VEC_SIZE);
	cout << vec.size()<<endl;
#pragma omp parallel for
	for (int i=0; i < VEC_SIZE; i++) {
		vec.push_back(i);
	}

	cout << "vector:";
	for (int i=0; i < VEC_SIZE; i++) {
		cout << vec[i];
	}
	cout << endl;
}

int test_svector()
{
	svector<int> vec(VEC_SIZE);
	cout << vec.size()<<endl;
#pragma omp parallel for
	for (int i=0; i < VEC_SIZE; i++) {
		vec.push_back(i);
	}

	for (int i=0; i < VEC_SIZE; i++) {
		verify_vec[vec[i]]++;
	}
	cout << "verify vector:";
	for (int i=0; i < VEC_SIZE; i++) {
		if (verify_vec[i] != 1) 
			cout << verify_vec[i] << " ";
	}
	cout << endl;
}

int main()
{
	memset(verify_vec, 0, sizeof(verify_vec));

	test_svector();
	
	return 0;
}
