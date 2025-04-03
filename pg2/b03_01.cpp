#include <iostream>

using namespace std;

int* func() {
	// the array d is a local variable, livetime ends on scope exit
	int d[3];
	int *dptr = &d[1];
	for (int i = 0; i < 3; ++i) {
		d[i] = 42;
		cout << d[1] << endl;
	}
	return dptr;
}

int* better_func() {
	// dynamic memory allocated, live does not end on exit, -> need to destroy
	int *d = new int[3];
	for (int i = 0; i < 3; ++i) {
		d[i] = 42;
		cout << d[1] << endl;
	}
	return d;
}


int main() {
	int *dptr = better_func();
	cout << "Hi" << endl;
	for (int i = 0; i < 3; ++i)
		cout << dptr[i] << endl;
	delete [] dptr;
	return 0;
}
