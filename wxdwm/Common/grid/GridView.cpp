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

#include "GridView.h"

GridView::GridView(Grid* model)
{
    if (model == NULL)
        m_model.reset(new Grid());
    else
        m_model.reset(model);
}

GridView::TRects GridView::rects(u32 x, u32 y, u32 width, u32 height)
{
    return _rects(x, y, width, height, m_model->tree());
}

bool GridView::resize(void* data, u32 old_width, u32 old_height, u32 width, u32 height)
{
    Grid::TNode::TPtr node = m_model->node(data);
    if (node == NULL)
        return false;

    Grid::TNode::TPtr parent = node->getParent();
    
    bool resize = false;
    if (old_width != width)
        resize |= _resize(parent, GridItem::eH, old_width, width, node->isLeft());
    if (old_height != height)
        resize |= _resize(parent, GridItem::eV, old_height, height, node->isLeft());
    return resize;
}

GridView::TRects GridView::_rects(u32 x, u32 y, u32 width, u32 height, Grid::TNode::TPtr root)
{
    TRects f;

    if (root->empty())
    {
        if (root->data.m_data == NULL) // is empty root
            return f;

        return TRects(1, SRect({ root->data.m_data, x, y, width, height }));
    }

    u16 orientation = root->data.m_orientation;
    if (orientation & GridItem::eAuto)
    {
        if (height <= width)
            orientation = GridItem::eAuto | GridItem::eH;
        else
            orientation = GridItem::eAuto | GridItem::eV;
        root->data.m_orientation = orientation;
    }

    u32 width1 = width;
    u32 width2 = width;
    u32 height1 = height;
    u32 height2 = height;
    u32 x2 = x;
    u32 y2 = y;

    if (orientation & GridItem::eH)
    {
        width1 *= root->data.m_proportion;
        width2 -= width1;
        x2 = width1;
    }
    else
    {
        height1 *= root->data.m_proportion;
        height2 -= height1;
        y2 = height1;
    }
    
    Grid::TNode::TPtr left = root->getLeft();
    Grid::TNode::TPtr right = root->getRight();
    if (left == NULL && right == NULL)
        return f;

    if (left == NULL || right == NULL)
        return _rects(x, y, width1 + width2, height1 + height2, (left == NULL ? right : left));

    f.splice(f.end(), _rects(x, y, width1, height1, left));
    f.splice(f.end(), _rects(x2, y2, width2, height2, right));
    return f;
}

bool GridView::_resize(Grid::TNode::TPtr parent, GridItem::EOrientation o, u32 old, u32 size, bool left)
{
    if (parent == NULL)
        return true;

    if ((parent->data.m_orientation & o) != o && parent->data.m_orientation != GridItem::eAuto)
        return _resize(parent->getParent(), o, old, size, parent->isLeft());

    double p = 0;
    if (left)
        p = parent->data.m_proportion;
    else
        p = 1 - parent->data.m_proportion;

    p = (size * p) / old;

    if (left)
        parent->data.m_proportion = p;
    else
        parent->data.m_proportion = 1 - p;

    return true;
}
