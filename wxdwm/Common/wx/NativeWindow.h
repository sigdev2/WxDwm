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

#include <atomic>
#include <unordered_set>

#include "wx/nativewin.h"

#define wxNWin NativeWindow

#ifndef String
#    ifndef UNICODE  
#        define String std::string
#    else
#        define String std::wstring
#    endif // UNICODE
#endif // String

class NativeWindow : public wxNativeContainerWindow
{
	struct SWindowStyle
	{
		LONG style;
		LONG extStyle;
		WINDOWPOS position;
		WINDOWPLACEMENT placement;
	};

	enum EConsts
	{
		eMinWindowSize = 3
	};

public:
	NativeWindow(HWND hwnd);
	virtual ~NativeWindow();

	virtual void Restore();
	virtual void Maximize(bool maximize = true);
	virtual bool IsMaximized() const { return IsMaximized(m_hWnd); }
	virtual void Minimize();
	virtual bool IsMinimized() const { return IsMinimized(m_hWnd); }
	
	void StorePosition();
	void RestorePosition(bool storeState = true);
	bool HasStorePosition() const { return m_pOpt != NULL; }
	bool IsInResize() const { return m_busyResize; }
    bool IsFullScreen() const { return IsFullScreen(m_hWnd); }

	void Rise() const { return Rise(m_hWnd); }

	wxRect GetRect() const { return GetRect(m_hWnd); }

	wxSize GetMinimumSize();

	bool isVisible(const wxRect& screen) const { return isVisible(m_hWnd, screen); }

    void CorrectRect(RECT* rect) const { return CorrectRect(m_hWnd, rect); }
    void CorrectRect(LONG* x, LONG* y, LONG* w, LONG* h) const { return CorrectRect(m_hWnd, x, y, w, h); }
    void DeCorrectRect(RECT* rect) const { return DeCorrectRect(m_hWnd, rect); }
    void DeCorrectRect(LONG* x, LONG* y, LONG* w, LONG* h) const { return DeCorrectRect(m_hWnd, x, y, w, h); }

    int TitleLength() const { return TitleLength(m_hWnd); }
    LONG Style() const { return Style(m_hWnd); }
    LONG ExStyle() const { return ExStyle(m_hWnd); }
    HWND Parent() const { return Parent(m_hWnd); }
    HWND Owner() const { return Owner(m_hWnd); }

    bool IsDisabled(LONG* style = NULL) const
        { return IsDisabled((style == NULL ? Style(m_hWnd) : *style)); }
    bool IsTool(LONG* exstyle = NULL) const
        { return IsTool((exstyle == NULL ? ExStyle(m_hWnd) : *exstyle)); }
    bool IsNoActivate(LONG* exstyle = NULL) const
        { return IsNoActivate((exstyle == NULL ? ExStyle(m_hWnd) : *exstyle)); }
    bool IsOverlapped(LONG* style = NULL, LONG* exstyle = NULL) const
        { return IsOverlapped((style == NULL ? Style(m_hWnd) : *style), (exstyle == NULL ? ExStyle(m_hWnd) : *exstyle)); }
    bool IsTransparent(LONG* exstyle = NULL) const
        { return IsTransparent((exstyle == NULL ? ExStyle(m_hWnd) : *exstyle)); }
    bool IsNoRender(LONG* exstyle = NULL) const
        { return IsNoRender((exstyle == NULL ? ExStyle(m_hWnd) : *exstyle)); }

    bool IsChildWindow() const { return IsChildWindow(m_hWnd); }

    bool IsSimpleWindow() const { return IsSimpleWindow(m_hWnd);  }
    unsigned long ProcessId() const { return ProcessId(m_hWnd); }
    bool IsParentFor(HWND child) const { return IsParentFor(m_hWnd, child); }
    bool IsChildFor(HWND parent) const { return IsChildFor(m_hWnd, parent); }

    String OwningUser() const { return OwningUser(m_hWnd); }

    HMONITOR Monitor() const { return Monitor(m_hWnd); }
    int      DisplayNumber() const { return DisplayNumber(m_hWnd); }

    void ToggleDecoration() const { return ToggleDecoration(m_hWnd); }
	String NativeWindow::GetExe(bool native = false) const { return NativeWindow::GetExe(m_hWnd, native); }

    // static

    static HWND GetActiveWindow() { return ::GetActiveWindow(); };

    static void Restore(HWND hwnd);
    static void Maximize(HWND hwnd, bool maximize = true);
    static bool IsMaximized(HWND hwnd);
    static void Minimize(HWND hwnd);
    static bool IsMinimized(HWND hwnd);

    static bool IsFullScreen(HWND hwnd);

    static void Rise(HWND hwnd);

    static wxRect GetRect(HWND hwnd);

    static wxSize GetMinimumSize(HWND hwnd);

    static bool isVisible(HWND hwnd, const wxRect& screen);

	static void CorrectRect(HWND hwnd, RECT* rect) { _correctRect(hwnd, rect); }
    static void CorrectRect(HWND hwnd, LONG* x, LONG* y, LONG* w, LONG* h);
    static void DeCorrectRect(HWND hwnd, RECT* rect) { _decorrectRect(hwnd, rect); }
    static void DeCorrectRect(HWND hwnd, LONG* x, LONG* y, LONG* w, LONG* h);

    static int TitleLength(HWND hwnd) { return ::GetWindowTextLength(hwnd); }
    static LONG Style(HWND hwnd) { return ::GetWindowLongPtr(hwnd, GWL_STYLE); }
    static LONG ExStyle(HWND hwnd) { return ::GetWindowLongPtr(hwnd, GWL_EXSTYLE); }
    static HWND Parent(HWND hwnd) { return ::GetParent(hwnd); }
    static HWND Owner(HWND hwnd) { return ::GetWindow(hwnd, GW_OWNER); }

    static bool IsDisabled(LONG style) { return style & WS_DISABLED; }
    static bool IsTool(LONG exstyle) { return exstyle & WS_EX_TOOLWINDOW; }
    static bool IsNoActivate(LONG exstyle) { return exstyle & WS_EX_NOACTIVATE; }
    static bool IsOverlapped(LONG style, LONG exstyle) // note: extoverlaped - is only hawe eadges 
        { return (style & WS_OVERLAPPEDWINDOW) && (true || (exstyle & WS_EX_OVERLAPPEDWINDOW)); }
    static bool IsTransparent(LONG exstyle) { return exstyle & WS_EX_TRANSPARENT; }
    static bool IsNoRender(LONG exstyle)
    {
#ifdef WS_EX_NOREDIRECTIONBITMAP
        return exstyle & WS_EX_NOREDIRECTIONBITMAP;
#else
        return false;
#endif // WS_EX_NOREDIRECTIONBITMAP
    }

    static bool IsChildWindow(HWND hwnd) { return Owner(hwnd) != NULL || Parent(hwnd) != NULL; }

    static bool IsSimpleWindow(HWND hwnd);
    static unsigned long ProcessId(HWND hwnd);
    static bool IsParentFor(HWND hwnd, HWND other);
    static bool IsChildFor(HWND hwnd, HWND other);

    static String OwningUser(HWND hwnd);

    static HMONITOR Monitor(HWND hwnd) { return ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST); }
    static int      DisplayNumber(HWND hwnd);

    static void ToggleDecoration(HWND hwnd);
	static String GetExe(HWND hwnd, bool native = false);

private:
	static void _correctRect(HWND hwnd, RECT* rect);
    static void _decorrectRect(HWND hwnd, RECT* rect);

	SWindowStyle* m_pOpt;
	wxSize* m_pMinimumSize;
    std::atomic_bool m_busyResize;
};

