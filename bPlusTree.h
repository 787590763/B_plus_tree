#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include<iostream>
#include"MACRO.h"
#include "data.h"
#include"BufferManager.h"

using namespace std;
extern BufferManager buff;
//extern char file[1000][BLOCK_LEN];

struct Block {
	char *b = NULL;
	void init(char flag, int parent){
		setFlag(flag);
		setParent(parent);
		//setPre(pre);
		//setNext(nxt);
	}
	char getFlag(){
		return b[0];
	}
	void setFlag(char flag){
		b[0] = flag;
	}
	int getNum(){
		return *(int *)(b + 1);
	}
	void setNum(int num){
		*(int *)(b + 1) = num;
	}
	int getParent(){
		return *(int *)(b + 5);
	}
	void setParent(int parent){
		*(int *)(b + 5) = parent;
	}
	int getNext(int itemSize){
		int temp = getNum();
		return *(int *)(b + 9 + temp * itemSize);
	}
	void setNext(int itemSize, int next){
		int temp = getNum();
		*(int *)(b + 9 + temp * itemSize) = next;
	}
	char *getKey(int itemSize, int n){
		return b + 13 + itemSize * n;
	}
	void setKey(char *key, int itemSize, int n){
		memcpy(b + 13 + itemSize * n, key, itemSize - 4);
	}
	int getKeyVal(int itemSize, int n){
		return *(int *)(b + 9 + itemSize * n);
	}
	void setKeyVal(int val, int n, int itemSize){
		*(int *)(b + 9 + itemSize * n) = val;
	}
	int getFree(){
		return *(int *)b;
	}
	void setFree(int n){
		*(int *)b = n;
	}
	void shiftRight(int n, int itemSize){
		memcpy(b + 9 + itemSize * (n + 1), b + 9 + itemSize * n, (getNum() - n) * itemSize + 4);
	}
	void shiftLeft(int n, int itemSize){
		memcpy(b + 9 + itemSize * n, b + 9 + itemSize * (n + 1), (getNum() - n - 1) * itemSize + 4);
	}
};

typedef struct keyInfo {
	Type keyType;
	int keyLength;
}keyInfo;

class BPlusTree {
public:
	string tableName;
	string keyName;
	int free;
	int maxn;
	int root;
	int len;
	int itemSize;
	keyInfo info;
	BPlusTree(){};
	BPlusTree(string tableName, string keyName, keyInfo info);
	~BPlusTree(){};
	Block *readCBlock(int node);
	Block *writeCBlock(int node);
	int cmp(char *a, char *b);
	int insert(int node, char *key, int tableNum);
	int insertNotFull(Block *block, char *key, int tableNum);
	int del(int node, char *key);
	void merge(Block *lblock, Block *rblock, char *key, int p);
	int deleteNotEmpty(Block *block, char *key);
	int getPos(int node, int parent);
	void fetchLeft(Block *lblock, Block *rblock, char *parentKey, int node);
	void fetchRight(Block *lblock, Block *rblock, char *parentKey, int node);
	int getFileBlock();
	void deleteFileBlock(int node);
	int getNextFree(int node);
	void setNextFree(int node, int nextNode);
public:
	int Insert(char *key, int tableNum);
	int Delete(char *key);
	int Search(char *key);int searchLeaf(int node, char *key);
	void output(int node);
	int getNext(int node);
	int getFirstLeaf();
};

#endif