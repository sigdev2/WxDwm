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

#include "GridStorer.h"

GridStorer::GridStorer()
{
    m_stored.reserve(10);
    m_stored.resize(10, TGrid());
}

void GridStorer::store(int idx, const TGrid& grid)
{
    if (idx < 0 || idx >= 10)
        return;
    m_stored[idx] = grid;
}

GridStorer::TGrid GridStorer::restore(int idx)
{
    if (idx < 0 || idx >= 10)
        return TGrid();

    return m_stored[idx];
}

void GridStorer::remove(HWND h)
{
    for (TGrids::iterator it = m_stored.begin(); it != m_stored.end(); ++it)
    {
        TGrid::iterator jt = it->find(h);
        if (jt != it->end())
            it->erase(jt);
    }
}
