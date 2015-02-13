/*
 * Vertical level printing for binary tree
 */

#ifndef _BINARY_TREE_H
#define _BINARY_TREE_H

#include <iostream>
#include <string>
#include <vector>
#include <list>

namespace chikku
{

template <typename T>
class RBinaryTree
{
public:
	RBinaryTree(T d)
	 : mData(d),
           mLeft(0),
	   mRight(0)
	{}

	~RBinaryTree()
	{
		delete mLeft;
		delete mRight;
	}

	void insert(T d)
	{
		if (d < mData) {
			if (!mLeft) {
				this->mLeft = new RBinaryTree(d);
			} else {
				this->mLeft->insert(d);
			}
		} else {
			if (!mRight) {
				this->mRight = new RBinaryTree(d);
			} else {
				this->mRight->insert(d);
			}
		}
	}
	
	std::list<T> * createLevels(int &numLevels, bool vertical=true) const
	{
		int maxLevel = 0;
		int minLevel = 0;
		getLevels(0, minLevel, maxLevel, vertical);

		numLevels = maxLevel-minLevel + 1;
		std::list<T> *levels = new std::list<T>[numLevels];
		createLevels(levels, 0, minLevel, maxLevel, vertical);
		return levels;
	}

	void levelPrint(bool vertical) const
	{
		int numLevels = 0;
		std::list<T> *levels = createLevels(numLevels, vertical);

		if (levels) {
			for (int i=numLevels-1; i>=0; --i) {
				std::cout << "( ";
				for (typename std::list<T>::iterator iter=levels[i].begin(); iter != levels[i].end(); ++iter) {
					std::cout << *iter << " ";
				}
				std::cout << " )";
			}
			delete [] levels;
		}
	}

	void print() const
	{
		std::cout << "( " << mData << " )";
		if (mLeft) {
			mLeft->print();
		}
		if (mRight) {
			mRight->print();
		}
	}

    T data() const
    {
        return mData;
    }

    std::list<RBinaryTree<T> *> reverse()
    {
        std::list<RBinaryTree<T> *> reversed;
        reverse(0, this, reversed);
        return reversed;
    }

private:
    void reverse(RBinaryTree<T> *parent, RBinaryTree<T> *root, std::list<RBinaryTree<T> *> & reversed)
    {
        if (!root) {
            return;
        }

        if (!root->mLeft && !root->mRight) {
            root->mRight = parent;
            reversed.push_back(root);
        } else if (root->mLeft && root->mRight) {
            reverse(root, root->mLeft, reversed);
            reverse(root, root->mRight, reversed);
            root->mRight = parent;
            root->mLeft = 0;
        } else if (root->mRight) {
            reverse(root, root->mRight, reversed);
            root->mRight = parent;
        } else if (root->mLeft) {
            reverse(root, root->mLeft, reversed);
            root->mLeft = parent;
        }
    }

	void createLevels(std::list<T> * levels, int level, int minLevel, int maxLevel, bool vertical) const
	{
		std::list<T> & listLevel = levels[level - minLevel];
		listLevel.insert(listLevel.end(), mData);

		if (mLeft) {
			int newLevel = level+1;
			if (vertical) {
				newLevel = level-1;
			}
			mLeft->createLevels(levels, newLevel, minLevel, maxLevel, vertical);
		}
		if (mRight) {
			mRight->createLevels(levels, level+1, minLevel, maxLevel, vertical);
		}
	}

	void getLevels(int level, int &minLevel, int &maxLevel, bool vertical) const
	{
		if (minLevel > level) {
			minLevel = level;
		}
		if (maxLevel < level) {
			maxLevel = level;
		}

		if (mLeft) {
			int newLevel = level+1;
			if (vertical) {
				newLevel = level-1;
			}
			mLeft->getLevels(newLevel, minLevel, maxLevel, vertical);
		}
		if (mRight) {
			mRight->getLevels(level+1, minLevel, maxLevel, vertical);
		}
	}

private:
	T mData;
	RBinaryTree<T> * mLeft;
	RBinaryTree<T> * mRight;
};

}

#endif

