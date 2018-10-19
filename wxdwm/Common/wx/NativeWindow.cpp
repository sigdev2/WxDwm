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

#include "NativeWindow.h"

#include <Lmcons.h>
#include <Psapi.h>

#include "wx/display.h"

#include "../win/DwmHelper.h"

NativeWindow::NativeWindow(HWND hwnd)
	: wxNativeContainerWindow(hwnd), m_pOpt(NULL), m_pMinimumSize(NULL)
{
	m_busyResize.store(false);
	StorePosition();
	GetMinimumSize();
}

NativeWindow::~NativeWindow()
{
	delete m_pOpt;
	delete m_pMinimumSize;
}

void NativeWindow::Restore()
{
    m_busyResize.store(true);
    Restore(m_hWnd);
    m_busyResize.store(false);
}

void NativeWindow::Restore(HWND hwnd)
{
	WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
	::GetWindowPlacement(hwnd, &placement);
	placement.showCmd = SW_RESTORE;
	::SetWindowPlacement(hwnd, &placement);
}

void NativeWindow::Maximize(bool maximize)
{
    m_busyResize.store(true);
    Maximize(m_hWnd, maximize);
    m_busyResize.store(false);
}

void NativeWindow::Maximize(HWND hwnd, bool maximize)
{
	/*WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
	::GetWindowPlacement(hwnd, &placement);
	placement.showCmd = SW_SHOWMAXIMIZED;
	::SetWindowPlacement(hwnd, &placement); */
	SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

bool NativeWindow::IsMaximized(HWND hwnd)
{
    WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
    ::GetWindowPlacement(hwnd, &placement);
    return SW_SHOWMAXIMIZED == placement.showCmd;
}

void NativeWindow::Minimize()
{
    m_busyResize.store(true);
    Minimize(m_hWnd);
    m_busyResize.store(false);
}

void NativeWindow::Minimize(HWND hwnd)
{
	WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
	/* ::GetWindowPlacement(m_hWnd, &placement);
	placement.showCmd = SW_SHOWMINIMIZED;
	::SetWindowPlacement(m_hWnd, &placement); */
    SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

bool NativeWindow::IsMinimized(HWND hwnd)
{
	WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
	::GetWindowPlacement(hwnd, &placement);
	return SW_SHOWMINIMIZED == placement.showCmd;
}

void NativeWindow::StorePosition()
{
	delete m_pOpt;

	LONG lStyle = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	LONG lExStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);

	WINDOWPLACEMENT plc;

	::GetWindowPlacement(m_hWnd, &plc);

	if (plc.showCmd == SW_SHOWMAXIMIZED)
	    Restore();

	const wxSize size = GetSize();
	const wxPoint pos = GetPosition();
	m_pOpt = new SWindowStyle({ lStyle, lExStyle, {
		m_hWnd,
		NULL,//GetTopWindow(hwnd),
		pos.x,
		pos.y,
		size.GetWidth(),
		size.GetHeight(),
		SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DEFERERASE }, plc });

	if (plc.showCmd == SW_SHOWMAXIMIZED)
	    Maximize();
}

void NativeWindow::RestorePosition(bool storeState)
{
	if (m_pOpt == NULL)
		return;

	if (!::IsWindowVisible(m_hWnd))
		return;

	const bool maximized = IsMaximized();
	const bool minimized = IsMinimized();

	Restore();
	m_busyResize.store(true);
	::SetWindowPos(m_pOpt->position.hwnd, m_pOpt->position.hwndInsertAfter,
		m_pOpt->position.x, // x
		m_pOpt->position.y,  // y
		m_pOpt->position.cx, // width
		m_pOpt->position.cy, // height
		m_pOpt->position.flags);
	::SetWindowPlacement(m_pOpt->position.hwnd, &(m_pOpt->placement));

	SetWindowLongPtr(m_pOpt->position.hwnd, GWL_STYLE, m_pOpt->style);
	SetWindowLongPtr(m_pOpt->position.hwnd, GWL_EXSTYLE, m_pOpt->extStyle);

	SetWindowPos(m_pOpt->position.hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	m_busyResize.store(false);

	if (storeState)
	{
		if (maximized)
			Maximize();
		if (minimized)
			Minimize();
	}

	delete m_pOpt;
	m_pOpt = NULL;
}

bool NativeWindow::IsFullScreen(HWND hwnd)
{
    wxRect rect = GetRect(hwnd);

    for (size_t i = 0, l = wxDisplay::GetCount(); i < l; i++)
    {
        wxDisplay display(i);
        if (display.GetGeometry() == rect)
            return true;
    }

    return false;
}

void NativeWindow::Rise(HWND hwnd)
{
	HWND hCurWnd = ::GetForegroundWindow();
	DWORD dwMyID = ::GetCurrentThreadId();
	DWORD dwCurID = ::GetWindowThreadProcessId(hCurWnd, NULL);
	::AttachThreadInput(dwCurID, dwMyID, TRUE);
	::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetForegroundWindow(hwnd);
	::AttachThreadInput(dwCurID, dwMyID, FALSE);
	::SetFocus(hwnd);
	::SetActiveWindow(hwnd);
}

wxRect NativeWindow::GetRect(HWND hwnd)
{
	RECT rect;
	::GetWindowRect(hwnd, &rect);
	_decorrectRect(hwnd, &rect);
	return wxRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

wxSize NativeWindow::GetMinimumSize()
{
	if (!::IsWindowVisible(m_hWnd))
		return wxSize(1, 1);

	if (m_pMinimumSize != NULL)
		return *m_pMinimumSize;

	m_busyResize.store(true);
    wxSize ret = GetMinimumSize(m_hWnd);
	m_busyResize.store(false);

	m_pMinimumSize = new wxSize(ret);
	return *m_pMinimumSize;
}

wxSize NativeWindow::GetMinimumSize(HWND hwnd)
{
    RECT nrc;

    MINMAXINFO mm;
    memset(&mm, 0, sizeof(MINMAXINFO));
    SendMessage(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&mm);

    nrc.left = 0;
    nrc.right = mm.ptMinTrackSize.x;
    nrc.top = 0;
    nrc.bottom = mm.ptMinTrackSize.y;


    return wxSize(nrc.right - nrc.left, nrc.bottom - nrc.top);
}

bool NativeWindow::isVisible(HWND hwnd, const wxRect& screen)
{
	if (!::IsWindowVisible(hwnd) || ::IsIconic(hwnd))
		return false;

	wxRect rect = GetRect(hwnd);

	if (rect.GetWidth() < eMinWindowSize || rect.GetHeight() < eMinWindowSize)
		return false;

	wxRect intersect = rect.Intersect(screen);

	if (intersect.GetWidth() < eMinWindowSize || intersect.GetHeight() < eMinWindowSize)
		return false;

	return true;
}

void NativeWindow::CorrectRect(HWND hwnd, LONG* x, LONG* y, LONG* w, LONG* h)
{
	RECT rect;
	rect.left = *x;
	rect.top = *y;
	rect.right = rect.left + *w;
	rect.bottom = rect.top + *h;
	
	_correctRect(hwnd, &rect);

	*x = rect.left;
	*y = rect.top;
	*w = rect.right - rect.left;
	*h = rect.bottom - rect.top;
}

void NativeWindow::DeCorrectRect(HWND hwnd, LONG* x, LONG* y, LONG* w, LONG* h)
{
	RECT rect;
	rect.left = *x;
	rect.top = *y;
	rect.right = rect.left + *w;
	rect.bottom = rect.top + *h;

	_decorrectRect(hwnd, &rect);

	*x = rect.left;
	*y = rect.top;
	*w = rect.right - rect.left;
	*h = rect.bottom - rect.top;
}

bool NativeWindow::IsSimpleWindow(HWND hwnd)
{
    LONG style = Style(hwnd);
    LONG exstyle = ExStyle(hwnd);
    if (IsChildWindow(hwnd) ||
        IsDisabled(style) ||
        IsTool(exstyle) ||
        IsNoActivate(exstyle) ||
        IsNoRender(exstyle) ||
        !IsOverlapped(style, exstyle) ||
        IsTransparent(exstyle))
        return false;
    return true;
}

unsigned long NativeWindow::ProcessId(HWND hwnd)
{
    unsigned long process_id = 0;
    GetWindowThreadProcessId(hwnd, &process_id);
    return process_id;
}

bool NativeWindow::IsParentFor(HWND hwnd, HWND other)
{
    if (hwnd == other)
        return true;

    HWND thisParent = NULL;
    std::unordered_set<HWND> visited;
    visited.insert(other);
    while ((thisParent = ::GetParent(other)) != NULL)
    {
        if (visited.find(thisParent) != visited.end())
            break;
        if (thisParent == hwnd)
            return true;
        visited.insert(thisParent);
    }

    return false;
}

bool NativeWindow::IsChildFor(HWND hwnd, HWND other)
{
    return IsParentFor(other, hwnd);
}

String NativeWindow::OwningUser(HWND hwnd)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessId(hwnd));

    if (hProcess == NULL)
        return String();

    DWORD cbuserInfo = sizeof(TOKEN_USER);
    HANDLE hToken;

    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
        return String();

    DWORD dwBufferSize = 0;
    GetTokenInformation(hToken,
        TokenUser,      // Request for a TOKEN_USER structure.  
        NULL,
        0,
        &dwBufferSize);

    PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, static_cast<SIZE_T>(dwBufferSize));
    if (!GetTokenInformation(hToken,
        TokenUser,
        pTokenUser,
        dwBufferSize,
        &dwBufferSize
    ))
    {
        CloseHandle(hToken);
        return String();
    }
    CloseHandle(hToken);

    if (IsValidSid(pTokenUser->User.Sid) == FALSE)
    {
        LocalFree(pTokenUser);
        return String();
    }

    PSID pSid = pTokenUser->User.Sid;
    TCHAR user[UNLEN + 1];
    TCHAR domain[UNLEN + 1];
    DWORD cbUser = UNLEN + 1;
    DWORD cbDomain = UNLEN + 1;
    SID_NAME_USE nu;
    if (LookupAccountSid(NULL, pSid, user, &cbUser, domain, &cbDomain, &nu) == FALSE)
    {
        LocalFree(pTokenUser);
        return String();
    }

    LocalFree(pTokenUser);

    return String(user);
}

int NativeWindow::DisplayNumber(HWND hwnd)
{
    const wxRect rc = GetRect(hwnd);
    const wxPoint center(rc.x + rc.width / 2, rc.y + rc.height / 2);
    return wxDisplay::GetFromPoint(center);
}

void NativeWindow::ToggleDecoration(HWND hwnd)
{
    LONG style = Style(hwnd);

    if ((style & WS_CAPTION) == WS_CAPTION)
        style &= ~WS_CAPTION;
    else
        style |= WS_CAPTION;

    SetWindowLong(hwnd, GWL_STYLE, style);
}

String NativeWindow::GetExe(HWND hwnd, bool native)
{
	TCHAR processName[MAX_PATH];
    HANDLE process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false,
        ProcessId(hwnd)
    );

    if (process == NULL)
        return String();

    GetModuleFileNameEx(process, NULL, processName, MAX_PATH);
    DWORD err = GetLastError();

    String str (processName);
    if (native)
	    return str;

    size_t index = 0;
    while (true)
    {
        index = str.find(_T('\\'), index);
        if (index == String::npos)
            break;
        str.replace(index, 1, _T("/"));
        index += 1;
    }

    return str;
}

void NativeWindow::_correctRect(HWND hwnd, RECT* rect)
{
    CDwmHelper::instance().correctRect(hwnd, rect);
}

void NativeWindow::_decorrectRect(HWND hwnd, RECT* rect)
{
    CDwmHelper::instance().decorrectRect(hwnd, rect);
}
