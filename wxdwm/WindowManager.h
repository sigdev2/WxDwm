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

#include <windows.h>

#include "wx/wx.h"

#include "Common/std/AtomicLocker.h"
#include "Common/std/numbers.h"
#include "Common/win/WinEventHook.h"
#include "Common/grid/GridView.h"

#include "Common/wx/GridStorer.h"
#include "Common/wx/NativeWindow.h"
#include "Common/wx/WindowsList.h"
#include "Common/wx/Displays.h"

#define __WMPtr(o) reinterpret_cast<WindowManager*>(o)
#define __WIN_GET_VISIBLE m_windows.getVisibleItems<TGridItems, GridItem>

class NativeWindow;

class WindowManager
{
public:
    typedef std::unordered_map<int, GridView::TSPtr> TViews;
    typedef std::unordered_map<HWND, std::pair<int, wxRect> > TMoveEventArgs;

    enum EConsts
    {
        eProcessRunDelay = 700
    };

	WindowManager(HWND parent);
    virtual ~WindowManager() {};

    // ents for windows list
	bool add(HWND hwnd);
	bool remove(HWND hwnd);
	bool active(HWND hwnd);

    // arrange mode
	bool arrange();
	void dearrange();

    // screen
    void updateScreen() { if (!m_arranged.locked()) arrange(); };
    void setScreenMode(u32 mode = Displays::eAll)
    { m_screen.setMode(mode); updateScreen(); }
    void toggleScreenMode(u32 mode = Displays::eAll)
    { m_screen.toggleMode(mode); updateScreen(); }

    // single window
    void orientation(u16 o = GridItem::eH);
    void toggleDecoration();

    // all windows
    void reflect(bool all = false);

    // window changed ewents
	void rearrange(HWND hwnd);
	void moveresizeStart(HWND hwnd);
    void moveresizeEnd(HWND hwnd);

    // 
	void draw(const GridView::TRects& rects) const;

    // store api
	void store(int idx);
    void restoreAll();
	bool restore(int idx); // note: return true if stored
    bool toggle(int idx); // note: return true if stored

    // enumerate windows
	static BOOL CALLBACK scan(HWND hwnd, LPARAM lParam);

    // serialize api
    String serialize();
    bool deserialize(const Json& data, bool run = false);

private:
	bool _isManageable(HWND hwnd);
    wxNWin* _removeFromView(HWND hwnd);
    void _restoreActive();

	HWND m_parent;
	wxNWin* m_active;
    TMoveEventArgs m_lastMoved;

	String m_user;

	AtomicFlag m_arranged;

	WinEventHook m_hook;

    Displays m_screen;
    WindowsList m_windows;

    TViews m_grid;
    GridStorer m_store;
};

