#include<iostream>
#include<algorithm>
#include<stdlib.h>
#include <ctime>
#include"BPlusTree.h"
using namespace std;

#define length 100

int main(){
	srand(unsigned(time(0)));
	keyInfo info;
	info.keyLength = 4;
	info.keyType = INT;
	int b[length];
	for (int i = 0; i < length; ++i){
		b[i] = i;
	}
	int t = rand() % 10 + 1;
	for (int i = 0; i < t; ++i){
		random_shuffle(b, b + length);
	}
	try{
		BPlusTree *tree = new BPlusTree("Student", "id", info);
		for (int i = 0; i < length; ++i){
//			cout << "insert " << i << " " << b[i] << endl;
			tree->Insert((char *)&b[i], b[i]);
		}
		/*for (int i = 0; i < tree->len; ++i){
			cout << i << ":" << endl;
			tree->output(i);
		}*/
		t = rand() % 10 + 1;
		for (int i = 0; i < t; ++i){
			random_shuffle(b, b + length);
		}
		for (int i = 0; i < length; ++i){
//			cout << "Delete " << b[i] << endl;
			tree->Delete((char *)&b[i]);
		}
		cout << "root:" << tree->root << endl;
		tree->output(tree->root);
		buff.flushBlocks();
		delete tree;
	}
	catch (string temp){
		cout << temp << endl;
	}
	system("pause");
	return 0;
}