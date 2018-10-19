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

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <windows.h>

#include "wx/wx.h"

#include "NativeWindow.h"

class WindowsList
{
public:
    typedef std::shared_ptr<NativeWindow> TWinPtr;
    typedef std::unordered_map<HWND, TWinPtr> TWindows;
    typedef std::unordered_set<NativeWindow*> TWidowsSet;

    WindowsList() {};
    virtual ~WindowsList();

    template<typename TList, typename TItem>
    TList getVisibleItems(const wxRect& screen, TWidowsSet* toReset = NULL);
    void add(HWND hwnd);
    void clear();
	bool empty() const { return m_windows.empty(); }
    bool has(HWND hwnd) const { return m_windows.find(hwnd) != m_windows.end(); }
    void remove(HWND hwnd);
    NativeWindow* get(HWND hwnd) const;

    std::vector<HWND> FindWindow(const String& path) const;
    std::vector<HWND> FindWindow(const String& path, long pid) const;
    
    void restoreAll(TWidowsSet* toReset = NULL);

    std::unordered_map<HWND, wxRect> visibleRects(const wxRect& screen) const;

private:
    bool _isVisible(NativeWindow* wnd, const wxRect& screen) const;

    TWindows m_windows;
};

template<typename TList, typename TItem>
TList WindowsList::getVisibleItems(const wxRect& screen, TWidowsSet* toReset)
{
    TGridItems ret;
    for (TWindows::iterator it = m_windows.begin(); it != m_windows.end(); ++it)
    {
        NativeWindow* wnd = it->second.get();
        if (_isVisible(wnd, screen))
        {
            if (!wnd->HasStorePosition())
                wnd->StorePosition();
            wnd->Restore();

            if (toReset != NULL)
            {
                TWidowsSet::iterator it = toReset->find(wnd);
                if (it != toReset->end())
                    toReset->erase(it);
            }

            ret.push_back(TItem(wnd));
        }
        else
        {
            if (toReset == NULL)
                wnd->RestorePosition();
            else
                toReset->insert(wnd);
        }
    }

    return ret;
}

