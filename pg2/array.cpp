#include <iostream>

using namespace std;

int main(int argc, char **argv) {
	int *array = nullptr;
	int len = 0;
	int input_value = 0;
	while (true) {
		cin >> input_value;
		if (cin.eof())
			break;
		if (!cin.good()) {
			cerr << "not good!\n";
			return 1;
		}
		len++;
		// make new array, fill with old, delete old
		// use identifier of old array to store adress of new
		int *new_array = new int[len];
		for (int i = 0; i < len; i++) {
			new_array[i] = array[i];
		}
		new_array[len-1] = input_value;
		delete [] array;
		array = new_array;
	}
}
