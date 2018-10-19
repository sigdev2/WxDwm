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

#include "Displays.h"

bool Displays::setMode(u32 mode)
{
    m_mode = mode;
    return update();
}

bool Displays::toggleMode(u32 mode)
{
    m_mode ^= mode;
    return update();
}

bool Displays::update()
{
    wxRect newScreen = _wxClientDisplayRect();
    if (newScreen != m_screen)
    {
        m_screen = newScreen;
        return true;
    }

    return false;
}

wxRect Displays::_wxClientDisplayRect() const
{
    wxRect rect;

    const size_t displysCount = wxDisplay::GetCount();
    for (size_t i = 0, l = displysCount; i < l; i++)
    {
        if (m_mode != eAll)
        {
            if ((m_mode & (0x1 << i)) == 0)
                continue;
        }

        wxDisplay display(i);
        wxRect r(display.GetClientArea());
        if (i == 0)
            rect = r;
        else
            rect = rect.Union(r);
    }

    return rect;
}

wxRect Displays::MonitorWorkArea(HMONITOR m)
{
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(m, &info);
    RECT& rect = info.rcWork;
    return wxRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

wxRect Displays::MonitorWorkArea(int m)
{
    wxDisplay display(m);
    return display.GetClientArea();
}
