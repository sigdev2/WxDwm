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

#include <unordered_map>

#include "../std/numbers.h"
#include "../std/Cache.h"
#include "../std/BinTree.h"

#define __DEFAULT_PROPORTION 0.5

class GridItem
{
    friend class Grid;
    friend class GridView;
public:
	enum EOrientation
	{
		eV = 0x1,
		eH = 0x2,
        eAuto = 0x4
	};

	GridItem(void* data = NULL, u16 orientation = eAuto, double proportion = __DEFAULT_PROPORTION)
		: m_data(data), m_proportion(proportion), m_orientation(orientation) {};
	virtual ~GridItem() {};

    bool operator==(const GridItem& other) const { return m_data == other.m_data; }
    bool operator!=(const GridItem& other) const { return m_data != other.m_data; }
    bool empty() const { return m_data == NULL; }
    void* getData() const { return m_data; }

protected:
	void* m_data;
    u16 m_orientation;
	double m_proportion;
};

typedef std::list<GridItem> TGridItems;

class Grid
{
    friend class GridView;
public:
	typedef BinTree<GridItem> TTree;
    typedef BinTreeNode<GridItem> TNode;
    typedef Cache<std::list<Grid::TNode::TPtr> > TListCahce;
    typedef CacheMap<void*, Grid::TNode::TPtr> TFindCahce;

    Grid()
    {
        m_tree = new BinTree<GridItem>(GridItem());
        m_cacheLists = new TListCahce([&]() { return this->m_tree->lists(); });
        m_cacheFind = new TFindCahce([&](void* data) { return this->m_tree->find(data); });
    };
    
    virtual ~Grid() { clear(); delete m_tree; delete m_cacheLists; delete m_cacheFind; };

    bool has(void* data) const { return (*m_cacheFind)(data) != NULL; }
    GridItem get(void* data) const;
    size_t size() const { std::list<Grid::TNode::TPtr> items = (*m_cacheLists)(); return items.size(); };
    // note: change parent settings
    void add(void* data, u16 orientation = GridItem::eAuto, double proportion = __DEFAULT_PROPORTION);
    void add(const GridItem& item);
    void add(const TGridItems& items);
    void swap(void* f, void* s);
    void replace(void* f, void* s);
    bool remove(void* data);
    // note: change parent settings
    void property(void* data, double proportion = __DEFAULT_PROPORTION);
    // note: change parent settings
    void property(void* data, u16 orientation = GridItem::eAuto);
    void clear() { m_tree->clear(); m_cacheLists->reset(); m_cacheFind->reset(); }
    TNode::TPtr node(void* data) const;

    TGridItems items() const;
    bool reflect(void* data = NULL);

protected:
    TTree* tree() const { return m_tree; };

private:
    void _add(const GridItem& item);

	TTree* m_tree;
    TListCahce* m_cacheLists;
    TFindCahce* m_cacheFind;
};

