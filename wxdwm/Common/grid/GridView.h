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

#include "Grid.h"

#include <memory>

class GridView
{
public:
	struct SRect
	{
		void* data;
		u32 x;
		u32 y;
		u32 width;
		u32 height;
	};

	typedef std::list<SRect> TRects;
    typedef std::shared_ptr<Grid> TGridPtr;
    typedef std::shared_ptr<GridView> TSPtr;

	GridView(Grid* model = NULL);
    virtual ~GridView() {};

    TGridPtr model() const { return m_model; }
    void setModel(Grid* model = NULL) { m_model.reset(model); }

    TRects rects(u32 x, u32 y, u32 width, u32 height);
    bool resize(void* data, u32 old_width, u32 old_height, u32 width, u32 height);

private:
    TRects _rects(u32 x, u32 y, u32 width, u32 height, Grid::TNode::TPtr root);
    bool _resize(Grid::TNode::TPtr parent, GridItem::EOrientation o, u32 old, u32 size, bool left);

    TGridPtr m_model;
};