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

#include <Lmcons.h>

#include <wx/tokenzr.h>
#include <wx/process.h>

#include "Common/wx/Config.h"

#include "WindowManager.h"

//todo: partical subtree redraw

WindowManager::WindowManager(HWND parent)
	: m_parent(parent), m_active(NULL),
	m_hook(this, NULL, EVENT_SYSTEM_MOVESIZESTART, EVENT_OBJECT_LOCATIONCHANGE)
{	
    // cache current user name
	DWORD username_len = UNLEN + 1;
	TCHAR username[UNLEN + 1];
	GetUserName(username, &username_len);
	m_user.append(username);

    // enumerate windows
	::EnumWindows(&WindowManager::scan, reinterpret_cast<LPARAM>(this));

    // window events
    m_hook.Add(EVENT_SYSTEM_MINIMIZEEND, WIN_HOOK_LAMBDA(__WMPtr(parent)->rearrange(hwnd)));
    m_hook.Add(EVENT_SYSTEM_MOVESIZESTART, WIN_HOOK_LAMBDA(__WMPtr(parent)->moveresizeStart(hwnd)));
    m_hook.Add(EVENT_SYSTEM_MOVESIZEEND, WIN_HOOK_LAMBDA(__WMPtr(parent)->moveresizeEnd(hwnd)));
    m_hook.Add(EVENT_OBJECT_LOCATIONCHANGE, WIN_HOOK_LAMBDA( \
        if (wxNWin::IsMaximized(hwnd)) \
            __WMPtr(parent)->rearrange(hwnd)));
}

bool WindowManager::add(HWND hwnd)
{
    if (!_isManageable(hwnd))
        return false;
    AtomicLocker lock(m_arranged);
    m_windows.add(hwnd);
	return true;
}

bool WindowManager::remove(HWND hwnd)
{
    _removeFromView(hwnd);
    m_store.remove(hwnd);
    m_windows.remove(hwnd);
	return true;
}

bool WindowManager::active(HWND hwnd)
{
	return (m_active = m_windows.get(hwnd)) == NULL;
}

bool WindowManager::arrange()
{
    if (!m_arranged.locked())
        m_arranged.lock();

    m_grid.clear();
    m_active = NULL;

	if (m_windows.empty())
		return false;

    const u32 mode = m_screen.mode();
    if (mode == 0)
    {
        m_windows.restoreAll();
        return false;
    }

    for (size_t i = 0, l = (size_t)Displays::count(); i < l; i++)
    {
        if ((mode & (0x1 << i)) != 0)
        {
            TViews::iterator it = m_grid.find(i);
            if (it == m_grid.end())
                m_grid[i] = GridView::TSPtr(new GridView());
        }
    }

    WindowsList::TWidowsSet toReset;
    for (TViews::iterator it = m_grid.begin(); it != m_grid.end(); ++it)
    {
        const wxRect rc = Displays::MonitorWorkArea(it->first);
        it->second->model()->add(__WIN_GET_VISIBLE(rc, &toReset));
        draw(it->second->rects(rc.x, rc.y, rc.width, rc.height));
    }

    m_windows.restoreAll(&toReset);

    m_arranged.unlock();
    _restoreActive();

    return true;
}

void WindowManager::dearrange()
{
    if (m_arranged.locked())
        return;

    m_arranged.lock();
    m_active = NULL;
    m_windows.restoreAll();
    m_grid.clear();
    _restoreActive();
}

void WindowManager::rearrange(HWND hwnd)
{
    if (m_arranged.locked())
        return;
    NativeWindow* wnd = _removeFromView(hwnd);
    if (wnd != NULL)
        wnd->RestorePosition();
}

void WindowManager::orientation(u16 o)
{
    if (m_active == NULL)
        return;

    int m = m_active->DisplayNumber();
    TViews::iterator it = m_grid.find(m);
    if (it != m_grid.end())
    {
        it->second->model()->property(m_active, o);
        const wxRect rc = Displays::MonitorWorkArea(m);
        draw(it->second->rects(rc.x, rc.y, rc.width, rc.height));
    }
}

void WindowManager::toggleDecoration()
{
    if (m_active == NULL)
        return;

    m_active->ToggleDecoration();
}

void WindowManager::reflect(bool all)
{
    if (m_arranged.locked())
        return;

    if (all)
    {
        for (TViews::iterator it = m_grid.begin(); it != m_grid.end(); ++it)
        {
            it->second->model()->reflect();
            const wxRect rc = Displays::MonitorWorkArea(it->first);
            draw(it->second->rects(rc.x, rc.y, rc.width, rc.height));
        }
    }
    else
    {
        int m = (m_active == NULL ? wxDisplay::GetFromPoint(wxGetMousePosition()) :
            m_active->DisplayNumber());

        TViews::iterator it = m_grid.find(m);
        if (it != m_grid.end())
        {
            it->second->model()->reflect(m_active);
            const wxRect rc = Displays::MonitorWorkArea(m);
            draw(it->second->rects(rc.x, rc.y, rc.width, rc.height));
        }
    }
}

void WindowManager::moveresizeStart(HWND hwnd)
{
    if (m_arranged.locked())
        return;

    NativeWindow* wnd = m_windows.get(hwnd);
    if (wnd == NULL || wnd->IsInResize())
        return;

    m_lastMoved[hwnd] = std::make_pair(wnd->DisplayNumber(), wnd->GetRect());
}

void WindowManager::moveresizeEnd(HWND hwnd)
{
    if (m_arranged.locked())
        return;

    TMoveEventArgs::iterator moveIt = m_lastMoved.find(hwnd);
    if (m_lastMoved.empty() || moveIt == m_lastMoved.end())
        return;

    NativeWindow* wnd = m_windows.get(hwnd);
    if (wnd == NULL || wnd->IsInResize())
        return;

    std::pair<int, wxRect> info = moveIt->second;
    m_lastMoved.clear();

    TViews::iterator view = m_grid.find(info.first);
    TViews::iterator current = m_grid.find(wnd->DisplayNumber());
    if (current == m_grid.end())
    {
        if (view == m_grid.end())
            return;
        view->second->model()->remove(wnd);
        const wxRect firstMonitor = Displays::MonitorWorkArea(view->first);
        draw(view->second->rects(firstMonitor.x, firstMonitor.y, firstMonitor.width, firstMonitor.height));
        return;
    }
    else if (view == m_grid.end())
    {
        current->second->model()->add(wnd);
        const wxRect secondMonitor = Displays::MonitorWorkArea(current->first);
        draw(current->second->rects(secondMonitor.x, secondMonitor.y, secondMonitor.width, secondMonitor.height));
        return;
    }

    AtomicLocker lock(m_arranged);

	const wxRect rc = wnd->GetRect();
	if (rc.height == info.second.height &&
		rc.width == info.second.width)
	{
        NativeWindow* second = NULL;
		GridView::TSPtr pView = view->second;
		GridView::TSPtr pCurrent = current->second;
        u64 max = 0;
        const TGridItems items = pCurrent->model()->items();
        for (TGridItems::const_iterator jt = items.begin(); jt != items.end(); ++jt)
        {
            NativeWindow* wndSecond = reinterpret_cast<NativeWindow*>(jt->getData());
            if (wndSecond == wnd)
                continue;

            const wxRect secondRc = wndSecond->GetRect();
            const wxRect intersec = rc.Intersect(secondRc);
            const u64 mult = intersec.height * intersec.width;
            if (mult > max)
            {
                max = mult;
                second = wndSecond;
            }
        }

        if (second == NULL)
        {
            arrange();
            return;
        }

        const wxRect monitor = Displays::MonitorWorkArea(current->first);
        if (pView.get() == pCurrent.get())
        {
			pCurrent->model()->swap(wnd, second);
        }
        else
        {
			pView->model()->replace(wnd, second);
            const wxRect firstMonitor = Displays::MonitorWorkArea(view->first);
            draw(pView->rects(firstMonitor.x, firstMonitor.y, firstMonitor.width, firstMonitor.height));

			pCurrent->model()->swap(second, wnd);
        }

        draw(pCurrent->rects(monitor.x, monitor.y, monitor.width, monitor.height));
	}
	else
	{
        for (TViews::iterator it = m_grid.begin(); it != m_grid.end(); ++it)
        {
            if (it->second->resize(wnd, info.second.width, info.second.height, rc.width, rc.height))
            {
                const wxRect monitor = Displays::MonitorWorkArea(it->first);
                draw(it->second->rects(monitor.x, monitor.y, monitor.width, monitor.height));
            }
        }
	}
}

void WindowManager::draw(const GridView::TRects& rects) const
{
	const i32 s = (i32)rects.size();

	if (s <= 0)
		return;

	HDWP positionStructure = BeginDeferWindowPos(s);

	for (GridView::TRects::const_iterator it = rects.begin(); it != rects.end(); ++it)
	{
        GridView::SRect rc = *it;
		NativeWindow* wnd = reinterpret_cast<NativeWindow*>(rc.data);
        if (wnd->IsMaximized() || wnd->IsMinimized())
            wnd->Restore();
		wnd->CorrectRect((LONG*)&(rc.x), (LONG*)&(rc.y), (LONG*)&(rc.width), (LONG*)&(rc.height));
		positionStructure = DeferWindowPos(positionStructure, wnd->GetHWND(),
			HWND_TOP, rc.x, rc.y, rc.width, rc.height,
			SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW);
		wnd->Rise();
	}

	EndDeferWindowPos(positionStructure);
}

void WindowManager::store(int idx)
{
    if (idx < 0 || idx >= 10)
        return;

    GridStorer::TGrid grid;
    for (int i = 0, l = Displays::count(); i < l; i++)
    {
        const wxRect monitor = Displays::MonitorWorkArea(i);
        GridStorer::TGrid other = m_windows.visibleRects(monitor);
        grid.insert(other.begin(), other.end());
    }

    m_store.store(idx, grid);
}

void WindowManager::restoreAll()
{
    GridStorer::TGrids& all = m_store.stored();
    GridView::TRects rects;
    for (GridStorer::TGrids::const_iterator it = all.begin(); it != all.end(); ++it)
    {
        for (GridStorer::TGrid::const_iterator jt = it->begin(); jt != it->end(); ++jt)
        {
            NativeWindow* wnd = m_windows.get(jt->first);
            const wxRect& rc = jt->second;
            if (wnd != NULL)
                rects.push_back(GridView::SRect({ wnd, (u32)rc.x, (u32)rc.y, (u32)rc.width, (u32)rc.height }));
        }

    }
    draw(rects);
}

bool WindowManager::restore(int idx)
{
    if (idx < 0 || idx >= 10)
        return false;

    GridStorer::TGrid grid = m_store.restore(idx);
    if (grid.size() <= 0)
    {
        store(idx);
        return true;
    }
    else
    {
        GridView::TRects rects;
        for (GridStorer::TGrid::const_iterator it = grid.begin(); it != grid.end(); ++it)
        {
            NativeWindow* wnd = m_windows.get(it->first);
            const wxRect& rc = it->second;
            if (wnd != NULL)
                rects.push_back(GridView::SRect({ wnd, (u32)rc.x, (u32)rc.y, (u32)rc.width, (u32)rc.height }));
        }

        draw(rects);
    }
    return false;
}

bool WindowManager::toggle(int idx)
{
    if (idx < 0 || idx >= 10)
        return false;

    GridStorer::TGrid grid = m_store.restore(idx);
    bool isRestore = grid.empty();
    if (!isRestore)
    {
        for (GridStorer::TGrid::const_iterator it = grid.begin(); it != grid.end(); ++it)
        {
            const NativeWindow* wnd = m_windows.get(it->first);
            wxRect tc = wnd->GetRect();
            if (wnd->GetRect() != it->second)
            {
                isRestore = true;
                break;
            }
        }
    }

    if (isRestore)
        return restore(idx);

    for (GridStorer::TGrid::const_iterator it = grid.begin(); it != grid.end(); ++it)
    {
        NativeWindow* wnd = m_windows.get(it->first);
        wnd->Minimize();
    }

    return false;
}


BOOL CALLBACK WindowManager::scan(HWND hwnd, LPARAM lParam)
{
    __WMPtr(lParam)->add(hwnd);
	return true;
}

String WindowManager::serialize()
{
    String out = _T("{ ");
    GridStorer::TGrids& stored = m_store.stored();
    size_t n = 0;
    for (GridStorer::TGrids::iterator it = stored.begin(); it != stored.end(); ++it)
    {
        if (it != stored.begin())
            out += _T(", ");

        out += _T("\"Screen") + wxString::Format(wxT("%i"), n) + _T("\" : { ");
        ++n;

        for (GridStorer::TGrid::iterator jt = it->begin(); jt != it->end(); ++jt)
        {
            wxRect& rect = jt->second;
            String name = wxNWin::GetExe(jt->first);
            if (name.empty())
                continue;

            if (jt != it->begin())
                out += _T(", ");
            out += _T("\"") + name + _T("\" : ");
            out += _T("\"");
            out += wxString::Format(wxT("%i"), rect.height) + _T(", ") +
                   wxString::Format(wxT("%i"), rect.width) + _T(", ") +
                   wxString::Format(wxT("%i"), rect.x) + _T(", ") +
                   wxString::Format(wxT("%i"), rect.y);
            out += _T("\"");
        }

        out += _T(" }");
    }

    return out + _T(" }");
}

bool WindowManager::deserialize(const Json& data, bool run)
{
    GridStorer::TGrids& stored = m_store.stored();
    size_t n = 0;
    bool has = false;
    for (GridStorer::TGrids::iterator it = stored.begin(); it != stored.end(); ++it)
    {
        const String key = _T("Screen") + wxString::Format(wxT("%i"), n);
        ++n;

        const Json& screen = data[__WStrToStr(key)];
        if (!screen.is_null())
        {
            const JsonObj& json_obj = screen.object_items();
            for (JsonObj::const_iterator jt = json_obj.begin(); jt != json_obj.end(); ++jt)
            {
                const std::string spath = jt->first;
                const String path(spath.begin(), spath.end());
                std::vector<HWND> list;
                if (run)
                {
                    list = m_windows.FindWindow(path);
                    wxProcess* process = wxProcess::Open(jt->first, wxEXEC_ASYNC | wxEXEC_NOHIDE);
                    if (process == NULL)
                        continue;
                    Thread::sleep(eProcessRunDelay);
                    std::vector<HWND> pidlist = m_windows.FindWindow(path, process->GetPid());

                    if (pidlist.size() <= 0)
                    {
                        pidlist = m_windows.FindWindow(path);
                        if (list.size() > 0)
                        {
                            for (std::vector<HWND>::iterator pIt = pidlist.begin(); pIt != pidlist.end(); ++pIt)
                            {
                                if (std::find(list.begin(), list.end(), *pIt) == list.end())
                                {
                                    pidlist.clear();
                                    pidlist.push_back(*pIt);
                                    break;
                                }
                            }
                        }
                    }
                    
                    list.swap(pidlist);
                }
                else
                {
                    list = m_windows.FindWindow(path);
                }

                if (list.size() > 0)
                {
                    wxStringTokenizer tokenizer(jt->second.string_value(), _T(", "));
                    int k = 0;
                    wxRect rc;
                    while (tokenizer.HasMoreTokens())
                    {
                        const wxString token = tokenizer.GetNextToken();
                        long value;
                        if (token.ToLong(&value))
                        {
                            switch (k)
                            {
                                case 0: rc.height = value; break;
                                case 1: rc.width = value; break;
                                case 2: rc.x = value; break;
                                case 3: rc.y = value; break;
                            }
                            ++k;
                        }
                    }

                    if (k == 4)
                    {
                        (*it)[list[0]] = rc;
                        has = true;
                    }
                }
            }
        }
    }

    return has;
}

bool WindowManager::_isManageable(HWND hwnd)
{
	if (hwnd == m_parent || m_windows.has(hwnd))
		return false;
	if (wxNWin::TitleLength(hwnd) <= 0)
		return false;
	if (wxNWin::IsChildWindow(hwnd))
		return false;
    if (!wxNWin::IsSimpleWindow(hwnd))
        return false;
    if (wxNWin::IsParentFor(hwnd, m_parent))
        return false;
	if (wxGetProcessId() == wxNWin::ProcessId(hwnd))
		return false;
    if (wxNWin::GetExe(hwnd).empty())
        return false;
    String user = wxNWin::OwningUser(hwnd);
	if (user.empty() || user != m_user)
		return false;

	return true;
}

wxNWin* WindowManager::_removeFromView(HWND hwnd)
{
    NativeWindow* wnd = m_windows.get(hwnd);
    if (wnd == NULL || wnd->IsInResize())
        return NULL;

    std::auto_ptr<AtomicLocker> lock(new AtomicLocker(m_arranged));

    if (wnd == m_active)
        m_active = NULL;

    if (!lock->before())
    {
        bool dearranged = true;
        std::list<int> toRemove;
        for (TViews::iterator it = m_grid.begin(); it != m_grid.end(); ++it)
        {
            std::shared_ptr<Grid> grid = it->second->model();
            if (grid->remove(wnd))
            {
                if (grid->size() > 0)
                {
                    dearranged = false;
                    const wxRect rc = Displays::MonitorWorkArea(it->first);
                    draw(it->second->rects(rc.x, rc.y, rc.width, rc.height));
                }
                else
                {
                    toRemove.push_back(it->first);
                }
            }
        }

        if (dearranged)
        {
            dearrange();
            lock.reset(NULL);
        }
        else
        {
            for (std::list<int>::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
                m_grid.erase(*it);
        }
    }

    _restoreActive();

    return wnd;
}

void WindowManager::_restoreActive()
{
    if (m_active != NULL)
        return;

    HWND active = NativeWindow::GetActiveWindow();
    if (active == NULL)
        return;

    NativeWindow* wnd = m_windows.get(active);
    if (wnd == NULL)
        return;

    m_active = wnd;
}