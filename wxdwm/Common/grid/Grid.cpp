// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#include "Grid.h"

GridItem Grid::get(void* data) const
{
    TNode::TPtr p = (*m_cacheFind)(data);
    return (p == NULL ? GridItem() : p->data);
}

void Grid::add(void* data, u16 orientation, double proportion)
{
    TNode::TPtr node = m_tree->addList(GridItem(data), GridItem());
    m_cacheLists->reset();
    m_cacheFind->reset();

    GridItem* parentData = node->parentData();

    if (parentData != NULL)
    {
        parentData->m_orientation = orientation;
        if (node->isLeft())
            parentData->m_proportion = proportion;
        else if (node->isRight())
            parentData->m_proportion =  1 - proportion;
    }
}

void Grid::add(const GridItem& item)
{
    m_cacheLists->reset();
    m_cacheFind->reset();
    _add(item);
}

void Grid::add(const TGridItems& items)
{
    m_cacheLists->reset();
    m_cacheFind->reset();
    for (TGridItems::const_iterator it = items.begin(); it != items.end(); ++it)
        _add(*it);
}

void Grid::swap(void* f, void* s)
{
    TNode::TPtr fItem = (*m_cacheFind)(f);
    TNode::TPtr sItem = (*m_cacheFind)(s);

    if (fItem == NULL || sItem == NULL)
        return;

    m_tree->swap(fItem, sItem);
}

void Grid::replace(void* f, void* s)
{
    TNode::TPtr fItem = (*m_cacheFind)(f);
    if (fItem == NULL)
        return;
    fItem->data = s;
}

bool Grid::remove(void* data)
{
    TNode::TPtr fItem = (*m_cacheFind)(data);
    if (fItem == NULL)
        return false;

    m_cacheLists->reset();
    m_cacheFind->reset();
    return m_tree->eraseList(fItem);
}

void Grid::property(void* data, double proportion)
{
    TNode::TPtr node = (*m_cacheFind)(data);
    if (node == NULL)
        return;

    TNode::TPtr parent = node->getParent();
    if (parent == NULL)
        node->data.m_proportion = proportion;
    else
        parent->data.m_proportion = proportion;
}

void Grid::property(void* data, u16 orientation)
{
    TNode::TPtr node = (*m_cacheFind)(data);
    if (node == NULL)
        return;

    TNode::TPtr parent = node->getParent();
    if (parent == NULL)
        node->data.m_orientation = orientation;
    else
        parent->data.m_orientation = orientation;
}

Grid::TNode::TPtr Grid::node(void* data) const
{
    return (*m_cacheFind)(data);
}

TGridItems Grid::items() const
{
    TGridItems i;
    std::list<Grid::TNode::TPtr> items = (*m_cacheLists)();
    for (std::list<Grid::TNode::TPtr>::const_iterator it = items.begin(); it != items.end(); ++it)
        i.push_back((*it)->data);

    return i;
}

bool Grid::reflect(void* data)
{
    if (data == NULL)
        return m_tree->reflect();

    TNode::TPtr node = (*m_cacheFind)(data);
    if (node == NULL)
        return m_tree->reflect();

    return m_tree->reflect(node->getParent());
}

void Grid::_add(const GridItem& item)
{
    TNode::TPtr node = m_tree->addList(item, GridItem());
}