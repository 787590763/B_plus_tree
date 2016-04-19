#include"bPlusTree.h"

BPlusTree::BPlusTree(string tableName, string keyName, keyInfo info):tableName(tableName), keyName(keyName){
	itemSize = info.keyLength + 4;
	this->info = info;
	maxn = (BLOCK_LEN - 13) / itemSize;
//	maxn = 4;
	free = 0;
	setNextFree(0, 1);
	len = 0;
	root = getFileBlock();
	Block *node = writeCBlock(root);
	node->init('!', -1);
	node->setNum(0);
	node->setNext(itemSize, -1);
	delete node;
}

Block *BPlusTree::readCBlock(int node){
	string fileName = tableName + keyName + INDEX_SUFFIX;
	Block *block = new Block();
	block->b = buff.readCBlock(fileName, node);
//	block->b = file[node];
	return block;
}

Block *BPlusTree::writeCBlock(int node){
	string fileName = tableName + keyName + INDEX_SUFFIX;
	Block *block = new Block();
	block->b = buff.writeCBlock(fileName, node);
//	block->b = file[node];
	return block;
}

int BPlusTree::cmp(char *a, char *b){
	if (info.keyType == CHAR || info.keyType == VCHAR){
		return strncmp(a, b, info.keyLength);
	}
	else if (info.keyType == INT){
		return *((int *)a) - *((int *)b);
	}
	else{
		return *((float *)a) - *((float *)b);
	}
}

int BPlusTree::Insert(char *key, int tableNum){
	int leaf = searchLeaf(root, key);
	return insert(leaf, key, tableNum);
}

int BPlusTree::insert(int node, char *key, int tableNum){
	Block *pnode = writeCBlock(node);
	char flag = pnode->getFlag();
	int Num = pnode->getNum();
	int parent = pnode->getParent();
	if (Num < maxn - 1){
		if (insertNotFull(pnode, key, tableNum)){
			delete pnode;
			return 1;
		}
		else{
			return 0;
		}
	}
	else{
		insertNotFull(pnode, key, tableNum);
		int newNode = getFileBlock();
		Block *pnew = writeCBlock(newNode);
		pnew->init(flag, parent);
		int half = (maxn + 1) / 2 - 1;
		pnode->setNum(half);
		char keyUp[200];
		memcpy(keyUp, pnode->getKey(itemSize, half), info.keyLength);
		if (flag == '!'){
			memcpy(pnew->b + 9, pnode->b + 9 + itemSize * half, itemSize * (maxn - half) + 4);
			pnew->setNum(maxn - half);
			pnode->setNext(itemSize, newNode);
		}
		else{
			memcpy(pnew->b + 9, pnode->b + 9 + itemSize * (half + 1), itemSize * (maxn - half - 1) + 4);
			pnew->setNum(maxn - half - 1);
			for (int i = 0; i <= maxn - half - 1; ++i){
				Block *pn = writeCBlock(pnew->getKeyVal(itemSize, i));
				pn->setParent(newNode);
				delete pn;
			}
		}
		if (parent == -1){
			parent = getFileBlock();
			root = parent;
			Block *pparent = writeCBlock(parent);
			pparent->init('?', -1);
			pparent->setNum(0);
			pparent->setKeyVal(node, 0, itemSize);
			pnew->setParent(parent);
			pnode->setParent(parent);
			delete pparent;
		}
		delete pnode;
		delete pnew;
		return insert(parent, keyUp, newNode);
	}
}

int BPlusTree::insertNotFull(Block *block, char *key, int tableNum){
	int Num = block->getNum();
	char flag = block->getFlag();
	int i;
	for (i = 0; i < Num; ++i){
		if (cmp(key, block->getKey(itemSize, i)) < 0)
			break;
		else if (cmp(key, block->getKey(itemSize, i)) == 0){
			cout << "Error: The index has been built!" << endl;
			return 0;
		}
	}
	block->shiftRight(i, itemSize);
	if (flag == '!'){
		block->setKey(key, itemSize, i);
		block->setKeyVal(tableNum, i, itemSize);
	}
	else{
		block->setKey(key, itemSize, i);
		block->setKeyVal(tableNum, i + 1, itemSize);
	}
	block->setNum(Num + 1);
	return 1;
}

int BPlusTree::Delete(char *key){
	int leaf = searchLeaf(root, key);
	return del(leaf, key);
}

int BPlusTree::del(int node, char *key){
	int i = 0;
	Block *pnode = writeCBlock(node);
	int parent = pnode->getParent();
	char flag = pnode->getFlag();
	int Num = pnode->getNum();
	if (Num == 0){
		return -1;
	}
	for (i = 0; i < Num; ++i){
		if (cmp(key, pnode->getKey(itemSize, i)) == 0){
			break;
		}
		else if (cmp(key, pnode->getKey(itemSize, i)) < 0){
			cout << "Error: Can't find the index!" << endl;
			return -1;
		}
	}
	if (i == Num){
		cout << "Error: Can't find the index!" << endl;
		return -1;
	}
	else{
		if (node == root){
			if (Num - 1 > 0 || flag == '!'){
				int temp = deleteNotEmpty(pnode, key);
				delete pnode;
				return temp;
			}
			int nroot = pnode->getKeyVal(itemSize, 0);
			Block *pn = writeCBlock(nroot);
			pn->setParent(-1);
			deleteFileBlock(root);
			root = nroot;
			delete pnode;
			delete pn;
			return 1;
		}
		if (Num >= (maxn + 1) / 2){
			int temp = deleteNotEmpty(pnode, key);
			delete pnode;
			return temp;
		}
		deleteNotEmpty(pnode, key);
		Block *plbrother, *prbrother;
		int current = getPos(node, parent);
		Block *pparent = readCBlock(parent);
		int pNum = pparent->getNum();
		int plb = -1, prb = -1;
		if (current > 0){
			plb = pparent->getKeyVal(itemSize, current - 1);
			char *parentKey = pparent->getKey(itemSize, current - 1);
			plbrother = writeCBlock(plb);
			if (plbrother->getNum() > (maxn + 1) / 2 - 1){
				fetchLeft(plbrother, pnode, parentKey, node);
				delete pparent;
				delete pnode;
				delete plbrother;
				return 1;
			}
			else{
				if (current < pNum){
					prb = pparent->getKeyVal(itemSize, current + 1);
					prbrother = writeCBlock(prb);
					if (prbrother->getNum() > (maxn + 1) / 2 - 1){
						parentKey = pparent->getKey(itemSize, current);
						fetchRight(pnode, prbrother, parentKey, node);
						delete pparent;
						delete pnode;
						delete prbrother;
						delete plbrother;
						return 1;
					}
				}
				merge(plbrother, pnode, parentKey, plb);
				deleteFileBlock(node);
				delete pparent;
				delete pnode;
				delete plbrother;
				return del(parent, parentKey);
			}
		}
		/*if (current > 0){
			int plb = pparent->getKeyVal(itemSize, current - 1);
			char *parentKey = pparent->getKey(itemSize, current - 1);
			plbrother = writeCBlock(plb);
			if (plbrother->getNum() == (maxn + 1) / 2 - 1){
			merge(plbrother, pnode, parentKey, plb);
			deleteFileBlock(node);
			delete pparent;
			delete pnode;
			delete plbrother;
			return del(parent, parentKey);
			}
			else{
			fetchLeft(plbrother, pnode, parentKey, node);
			delete pparent;
			delete pnode;
			delete plbrother;
			return 1;
			}
			}*/
		else{
			prb = pparent->getKeyVal(itemSize, current + 1);
			char *parentKey = pparent->getKey(itemSize, current);
			prbrother = writeCBlock(prb);
			if (prbrother->getNum() == (maxn + 1) / 2 - 1){
				merge(pnode, prbrother, parentKey, node);
				deleteFileBlock(prb);
				delete pparent;
				delete pnode;
				delete prbrother;
				return del(parent, parentKey);
			}
			else{
				fetchRight(pnode, prbrother, parentKey, node);
				delete pparent;
				delete pnode;
				delete prbrother;
				return 1;
			}
		}
	}
}

void BPlusTree::merge(Block *lblock, Block *rblock, char *key, int p){
	int lNum = lblock->getNum();
	int rNum = rblock->getNum();
	int flag = lblock->getFlag();
	if (flag == '!'){
		lblock->setNum(lNum + rNum);
		for (int i = lNum; i < lNum + rNum; ++i){
			lblock->setKeyVal(rblock->getKeyVal(itemSize, i - lNum), i, itemSize);
			lblock->setKey(rblock->getKey(itemSize, i - lNum), itemSize, i);
		}
		lblock->setNext(itemSize, rblock->getNext(itemSize));
	}
	else{
		lblock->setNum(lNum + rNum + 1);
		lblock->setKey(key, itemSize, lNum);
		for (int i = lNum + 1; i < lNum + rNum + 1; ++i){
			lblock->setKeyVal(rblock->getKeyVal(itemSize, i - lNum - 1), i, itemSize);
			lblock->setKey(rblock->getKey(itemSize, i - lNum - 1), itemSize, i);
		}
		lblock->setNext(itemSize, rblock->getNext(itemSize));
		for (int i = lNum + 1; i < lNum + rNum + 1; ++i){
			Block *pn = writeCBlock(lblock->getKeyVal(itemSize, i));
			pn->setParent(p);
			delete pn;
		}
		Block *pn = writeCBlock(lblock->getKeyVal(itemSize, lNum + rNum + 1));
		pn->setParent(p);
		delete pn;
	}
}

int BPlusTree::deleteNotEmpty(Block *block, char *key){
	int Num = block->getNum();
	char flag = block->getFlag();
	int i;
	for (i = 0; i < Num; ++i){
		if (cmp(key, block->getKey(itemSize, i)) == 0){
			break;
		}
		else if (cmp(key, block->getKey(itemSize, i)) < 0){
			return -1;
		}
	}
	if (flag != '!'){
		block->setKeyVal(block->getKeyVal(itemSize, i), i + 1, itemSize);
	}
	block->shiftLeft(i, itemSize);
	block->setNum(Num - 1);
	return 1;
}

int BPlusTree::getPos(int node, int parent){
	Block *pnode = readCBlock(parent);
	int Num = pnode->getNum();
	for (int i = 0; i <= Num; ++i){
		if (pnode->getKeyVal(itemSize, i) == node){
			return i;
		}
	}
	return -1;
}

void BPlusTree::fetchLeft(Block *lblock, Block *rblock, char *parentKey, int node){
	char flag = lblock->getFlag();
	int lNum = lblock->getNum();
	int rNum = rblock->getNum();
	rblock->shiftRight(0, itemSize);
	if (flag == '!'){
		char *key = lblock->getKey(itemSize, lNum - 1);
		int val = lblock->getKeyVal(itemSize, lNum - 1);
		rblock->setKey(key, itemSize, 0);
		rblock->setKeyVal(val, 0, itemSize);
		int next = lblock->getNext(itemSize);
		lblock->setNum(lNum - 1);
		lblock->setNext(itemSize, next);
		memcpy(parentKey, key, info.keyLength);
	}
	else{
		int val = lblock->getNext(itemSize);
		char *key = lblock->getKey(itemSize, lNum - 1);
		rblock->setKey(parentKey, itemSize, 0);
		rblock->setKeyVal(val, 0, itemSize);
		Block *pn = writeCBlock(val);
		pn->setParent(node);
		delete pn;
		memcpy(parentKey, key, info.keyLength);
		lblock->setNum(lNum - 1);
	}
	rblock->setNum(rNum + 1);
}

void BPlusTree::fetchRight(Block *lblock, Block *rblock, char *parentKey, int node){
	char flag = lblock->getFlag();
	int rNum = rblock->getNum();
	int lNum = lblock->getNum();
	if (flag == '!'){
		char *key = rblock->getKey(itemSize, 0);
		int val = rblock->getKeyVal(itemSize, 0);
		int next = lblock->getNext(itemSize);
		lblock->setKey(key, itemSize, lNum);
		lblock->setKeyVal(val, lNum, itemSize);
		lblock->setNum(lNum + 1);
		lblock->setNext(itemSize, next);
		rblock->shiftLeft(0, itemSize);
		rblock->setNum(rNum - 1);
		memcpy(parentKey, rblock->getKey(itemSize, 0), info.keyLength);
	}
	else{
		char *key = rblock->getKey(itemSize, 0);
		int val = rblock->getKeyVal(itemSize, 0);
		lblock->setKey(parentKey, itemSize, lNum);
		lblock->setKeyVal(val, lNum + 1, itemSize);
		lblock->setNum(lNum + 1);
		Block *pn = writeCBlock(val);
		pn->setParent(node);
		delete pn;
		memcpy(parentKey, key, info.keyLength);
		rblock->shiftLeft(0, itemSize);
		rblock->setNum(rNum - 1);
	}
	
}

int BPlusTree::searchLeaf(int node, char *key){
	Block *pnode = readCBlock(node);
	if (pnode->getFlag() == '!'){
		return node;
	}
	int Num = pnode->getNum();
	for (int i = 0; i < Num; ++i){
		if (cmp(key, pnode->getKey(itemSize, i)) < 0){
			int nextNode = pnode->getKeyVal(itemSize, i);
			delete pnode;
			return searchLeaf(nextNode, key);
		}
	}
	int nextNode = pnode->getNext(itemSize);
	delete pnode;
	return searchLeaf(nextNode, key);
}

int BPlusTree::Search(char *key){
	int node;
	int leaf = searchLeaf(root, key);
	Block *pnode = readCBlock(leaf);
	int Num = pnode->getNum();
	if (Num == 0){
		return -1;
	}
	for (int i = 0; i < Num; ++i){
		if (cmp(key, pnode->getKey(itemSize, i)) < 0){
			delete pnode;
			return -1;
		}
		else if (cmp(key, pnode->getKey(itemSize, i)) == 0){
			node = pnode->getKeyVal(itemSize, i);
			break;
		}
	}
	delete pnode;
	return node;
}

int BPlusTree::getFileBlock(){
	int tmp = free;
	free = getNextFree(free);
	if (free > len){
		setNextFree(free, free + 1);
		len = free;
	}
	return tmp;
}

void BPlusTree::deleteFileBlock(int node){
	int tmp = free;
	free = node;
	setNextFree(free, tmp);
}

int BPlusTree::getNextFree(int node){
	
	Block *t = readCBlock(node);
	int temp = t->getFree();
	delete t;
	return temp;
}

void BPlusTree::setNextFree(int node, int nextFree){
	
	Block *t = writeCBlock(node);
	t->setFree(nextFree);
	delete t;
}

void BPlusTree::output(int node){
	Block *block = readCBlock(node);
	cout << block->getFlag() << " " << block->getNum() << " " << block->getParent() << endl;
	int Num = block->getNum();
	for (int i = 0; i < Num; ++i){
		cout << block->getKeyVal(itemSize, i) << " " << *(int *)block->getKey(itemSize, i) << " ";
	}
	cout << block->getNext(itemSize) << endl;
	delete block;
}

int BPlusTree::getNext(int node){
	Block *block = readCBlock(node);
	int temp = block->getNext(itemSize);
	delete block;
	return temp;
}

int BPlusTree::getFirstLeaf(){
	int temp;
	Block *block = readCBlock(root);
	while (block->getFlag() != '!'){
		temp = block->getKeyVal(itemSize, 0);
		delete block;
		block = readCBlock(temp);
	}
	delete block;
	return temp;
}