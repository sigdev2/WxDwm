/* Copyright 2018 SigDev

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include <list>

#include "numbers.h"

template<class T = void*>
class BinTreeNode
{
public:
    typedef BinTreeNode<T> TBase;
    typedef BinTreeNode<T>* TPtr;

	BinTreeNode(const T& d, TPtr p = NULL)
		: data(d), left(NULL), right(NULL), size(1), parent(p)
	{}

    virtual ~BinTreeNode()
    {
        TPtr p = parent;
        while (p != NULL)
        {
            p->size -= size;
            p = p->parent;
        }

        if (isLeft())
            parent->left = NULL;
        else if (isRight())
            parent->right = NULL;

        parent = NULL;
        deleteLeft();
        deleteRight();
    }

    TPtr find(const T& val)
    {
        if (val == data)
            return this;

        TPtr ret = NULL;
        if (left != NULL)
            ret = left->find(val);
        if (ret == NULL && right != NULL)
            ret = right->find(val);

        return ret;
    }

    std::list<TPtr> lists(TPtr root = NULL)
    {
        if (root == NULL)
            root = this;

        std::list<TPtr> l;

        if (root->particalEmpty())
            l.push_back(root);
        if (root->left != NULL)
            l.splice(l.end(), lists(root->left));
        if (root->right != NULL)
            l.splice(l.end(), lists(root->right));

        return l;
    }

    TPtr setLeft(const T& val)
    {
        if (left == NULL)
            setLeft(new TBase(val));
        else
            left->data = val;

        return left;
    }

    TPtr setRight(const T& val)
	{
        if (right == NULL)
            setRight(new TBase(val));
        else
            right->data = val;
        
        return right;
    }

    TPtr setLeft(TPtr node) { return _set(true, node); }
    TPtr setRight(TPtr node) { return _set(false, node); }

    void reflect()
    {
        std::swap(left, right);
    }

    void cutParent() { parent = NULL; }

    TPtr add(const T& val)
	{
		if (left == NULL)
			return setLeft(val);
		else if (right == NULL)
			return setRight(val);
        return NULL;
	}
	
    TPtr add(TPtr node)
	{
		if (left == NULL)
			return setLeft(node);
		else if (right == NULL)
			return setRight(node);
        return NULL;
	}

    TPtr get()
    {
        if (right != NULL)
            return right;
        else if (left != NULL)
            return left;
        return NULL;
    }

    void deleteLeft()
    {
        delete left;
        left = NULL;
    }
    void deleteRight()
    {
        delete right;
        right = NULL;
    }

    void clear() { deleteRight(); deleteLeft();  }

    bool empty() { return left == NULL && right == NULL; }
    bool particalEmpty() { return left == NULL || right == NULL; }

    bool isLeft() const { if (parent == NULL) return false;
                          return parent->left == this; }
    bool isRight() const { if (parent == NULL) return false;
                           return parent->right == this; }

    T* parentData() const { return (parent == NULL ? NULL : &parent->data); }

    TPtr getParent() const { return parent; }
    TPtr getLeft() const { return left; }
    TPtr getRight() const { return right; }

    TPtr siblingLeft(TPtr twig) { return _sibling(true, twig); }
    TPtr siblingRight(TPtr twig) { return _sibling(false, twig); }
    TPtr sibling(bool l, TPtr twig)
    {
        if (twig == NULL || twig->parent == NULL)
            return NULL;

        TPtr nodeParent = twig->parent;
        if (l)
        {
            if (twig == nodeParent->right.get() && nodeParent->left.get() != NULL)
                return nodeParent->left.get();
            else if (twig != nodeParent->left.get())
                return NULL;
        }
        else
        {
            if (twig == nodeParent->left.get() && nodeParent->right.get() != NULL)
                return nodeParent->right.get();
            else if (twig != nodeParent->right.get())
                return NULL;
        }

        TPtr p = _sibling(l, nodeParent);

        if (p == NULL)
            return NULL;

        if (l)
        {
            if (p->right.get() != NULL)
                return p->right.get();
            else if (p->left.get() != NULL)
                return p->left.get();
        }
        else
        {
            if (p->left.get() != NULL)
                return p->left.get();
            else if (p->right.get() != NULL)
                return p->right.get();
        }

        return _sibling(l, p);
    }

    size_t count() const { return size; }

	T data;

protected:
    TPtr parent;
    TPtr left;
    TPtr right;
    size_t size;

private:
	// node: replace node and delelte all other childs nodes
    TPtr _set(bool l, TPtr node)
	{
        if (node == NULL)
        {
            (l ? deleteLeft() : deleteRight());
            return NULL;
        }

		node->parent = this;

        if (l)
        {
            if (left != NULL)
                deleteLeft();
            left = node;
        }
        else
        {

            if (right != NULL)
                deleteRight();
            right = node;
        }

        TPtr p = this;
        while (p != NULL)
        {
            p->size += node->size;
            p = p->parent;
        }

        return (l ? left : right);
    }
};

template<class T = void*>
class BinTree : public BinTreeNode<T>
{
public:
    typedef BinTreeNode<T> TBase;

    BinTree(const T& rootVal) : TBase(rootVal) { }
    virtual ~BinTree() {}

    TPtr add(const T& val)
    {
        TPtr ptr = TBase::add(val);
        if (ptr == NULL)
        {
            ptr = ((BinTree*)left.get())->add(val);
            if (ptr == NULL)
                ptr = ((BinTree*)right.get())->add(val);
        }

        return ptr;
    }

    TPtr addList(const T& val, const T& null)
    {
        if (particalEmpty())
        {
            if (data == null)
            {
                return TBase::add(val);
            }
            else
            {
                TBase::add(data);
                data = null;
                if (particalEmpty())
                    return TBase::add(val);
            }
        }

        if (left->count() <= right->count())
            return ((BinTree*)left)->addList(val, null);

        return ((BinTree*)right)->addList(val, null);
    }

    void build(const std::list<T>& nodes)
    {
        for (std::list<T> it = nodes.begin(); it != nodes.end(); ++it)
            add(*it);
    }

    void buildLists(const std::list<T>& nodes, const T& null)
    {
        for (std::list<T> it = nodes.begin(); it != nodes.end(); ++it)
            addList(*it);
    }

    void swap(TPtr f, TPtr s)
    {
        if (f == NULL || s == NULL)
            return;

        std::swap(f->data, s->data);
    }

    bool erase(TPtr i)
    {
        TPtr p = i->getParent();
        if (p == NULL)
            return false;

        if (p->getLeft() == i)
            p->deleteLeft();
        else if (p->getRight() == i)
            p->deleteRight();
        else
            return false;

        return true;
    }

    bool eraseList(TPtr i, bool reduce = true)
    {
        if (!i->empty())
            return false;

        TPtr p = i->getParent();
        if (!erase(i))
            return false;

        if (p->empty())
            return true || eraseList(p, reduce);

        swap(p, p->get());
        p->clear();

        return true;
    }

    bool reflect(TPtr i = NULL)
    {
        if (i == NULL)
        {
            TBase::reflect();
            return true;
        }
        else if (i == this)
        {
            TBase::reflect();
            return true;
        }
        else if (reflect(left))
        {
            return true;
        }

        return reflect(right);
    }
};

