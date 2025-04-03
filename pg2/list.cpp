#include <iostream>

using namespace std;

struct int_list {
	int_list *next;
	int value;
};

int main(int argc, char **argv) {
	int_list *list = nullptr;
	
	int input_value = 0;
	while (true) {
		cin >> input_value;
		if (cin.eof())
			break;
		if (!cin.good()) {
			cerr << "not good!\n";
			return 1;
		}
		int_list *new_head = new int_list;
		new_head->next = list;
		list = new_head;
		new_head->value = input_value;
	}
	for (int_list *iter = list; iter; iter = iter->next) {
		cout << "value: " << iter->value << endl;
	}

	// delete list
	while (list) {
		int_list *to_delete = list;
		list = list->next;
		delete to_delete;
	}
}
