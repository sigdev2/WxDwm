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

#include "WindowsList.h"

WindowsList::~WindowsList()
{
    clear();
}

void WindowsList::add(HWND hwnd)
{
    NativeWindow* wnd = new NativeWindow(hwnd);
    if (wnd->GetHWND() != hwnd)
        delete wnd;

    m_windows[hwnd] = TWinPtr(wnd);
}

void WindowsList::clear()
{
    for (TWindows::iterator it = m_windows.begin(); it != m_windows.end(); ++it)
    {
        TWinPtr wnd = it->second;
        wnd->RestorePosition();
        wnd.reset();
    }
    m_windows.clear();
}

void WindowsList::remove(HWND hwnd)
{
    TWindows::iterator it = m_windows.find(hwnd);
    if (it != m_windows.end())
    {
        it->second.reset();
        m_windows.erase(it);
    }
}

NativeWindow* WindowsList::get(HWND hwnd) const
{
    TWindows::const_iterator it = m_windows.find(hwnd);
    if (it != m_windows.end())
        return it->second.get();

    return NULL;
}

std::vector<HWND> WindowsList::FindWindow(const String& path) const
{
    std::vector<HWND> hwnds;
    hwnds.reserve(2);

    for (TWindows::const_iterator it = m_windows.begin(); it != m_windows.end(); ++it)
    {
        String name = it->second->GetExe();
        if (!name.empty() && name == path)
            hwnds.push_back(it->first);
    }

    return hwnds;
}

std::vector<HWND> WindowsList::FindWindow(const String& path, long pid) const
{
    std::vector<HWND> hwnds;
    hwnds.reserve(2);

    for (TWindows::const_iterator it = m_windows.begin(); it != m_windows.end(); ++it)
    {
        String name = it->second->GetExe();
        if (!name.empty() && name == path && it->second->ProcessId() == pid)
            hwnds.push_back(it->first);
    }

    return hwnds;
}

void WindowsList::restoreAll(TWidowsSet* toReset)
{
    if (toReset == NULL)
        for (TWindows::iterator it = m_windows.begin(); it != m_windows.end(); ++it)
            it->second->RestorePosition();
    else
        for (TWidowsSet::iterator it = toReset->begin(); it != toReset->end(); ++it)
            (*it)->RestorePosition();
}

std::unordered_map<HWND, wxRect> WindowsList::visibleRects(const wxRect& screen) const
{
    std::unordered_map<HWND, wxRect> ret;
    for (TWindows::const_iterator it = m_windows.begin(); it != m_windows.end(); ++it)
    {
        TWinPtr wnd = it->second;
        if (_isVisible(wnd.get(), screen))
            ret[wnd->GetHWND()] = wnd->GetRect();
    }

    return ret;
}

bool WindowsList::_isVisible(NativeWindow* wnd, const wxRect& screen) const
{
    return wnd != NULL && wnd->isVisible(screen) && !wnd->IsMaximized() && !wnd->IsMinimized() && !wnd->IsFullScreen();
}