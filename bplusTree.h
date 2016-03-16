#ifdef BPLUSTREE_H
#define BPLUSTREE_H

#include<iostream>
#include<vector>
#include<string>
#include<cstdlib>
#include<cstdio>
#include<cassert>
#include"MACRO.h"
using namespace std;
#define Out(x) cout << #x"=" << x << endl;

struct NodeInfo {
	Type keyType;
	int keyLength;
}

struct Bplustree {
public:
	string stableName;
	string skeyName;
	int smaxn;
	int sfree;
	int sroot;
//	int slen;
	NodeInfo sinfo;
	Bplustree();	
	Bplustree(string tableName, string keyName, NodeInfo ninfo);
	~Bplustree();
	int Insert(char *key, int tableNum);
//	int Delete(char *key);
	int Search(char *key);
	int searchPos(int pos, char *key);
	int splitChild(int parent, int i);
//	int mergeChild(int parent, int i);
	int insertNotFull(int pos, char *key, int tableNum);
	int 
