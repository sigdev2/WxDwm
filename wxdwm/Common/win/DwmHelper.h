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

#include <windows.h>

#include "../std/Singleton.h"

class CDwmHelper : public singleton<CDwmHelper>
{
public:
	typedef enum _DWMWINDOWATTRIBUTE
	{
		DWMWA_NCRENDERING_ENABLED = 1,
		DWMWA_NCRENDERING_POLICY,
		DWMWA_TRANSITIONS_FORCEDISABLED,
		DWMWA_ALLOW_NCPAINT,
		DWMWA_CAPTION_BUTTON_BOUNDS,
		DWMWA_NONCLIENT_RTL_LAYOUT,
		DWMWA_FORCE_ICONIC_REPRESENTATION,
		DWMWA_FLIP3D_POLICY,
		DWMWA_EXTENDED_FRAME_BOUNDS,
		DWMWA_HAS_ICONIC_BITMAP,
		DWMWA_DISALLOW_PEEK,
		DWMWA_EXCLUDED_FROM_PEEK,
		DWMWA_CLOAK,
		DWMWA_CLOAKED,
		DWMWA_FREEZE_REPRESENTATION,
		DWMWA_LAST
	} DWMWINDOWATTRIBUTE;

	CDwmHelper();
	virtual ~CDwmHelper();

	bool IsDwm() const;
	bool IsDwmAllowed() const;

	typedef HRESULT(WINAPI* DwmIsCompositionEnabled_t)(BOOL *pfEnabled);
	DwmIsCompositionEnabled_t DwmIsCompositionEnabled;
	typedef HRESULT(WINAPI* DwmSetWindowAttribute_t)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
	DwmSetWindowAttribute_t DwmSetWindowAttribute;
	typedef HRESULT(WINAPI* DwmExtendFrameIntoClientArea_t)(HWND hWnd, void* pMarInset);
	DwmExtendFrameIntoClientArea_t DwmExtendFrameIntoClientArea;
	typedef BOOL(WINAPI* DwmDefWindowProc_t)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
	DwmDefWindowProc_t DwmDefWindowProc;
	typedef HRESULT(WINAPI* DwmGetWindowAttribute_t)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
	DwmGetWindowAttribute_t DwmGetWindowAttribute;
	typedef HRESULT(WINAPI* DwmSetIconicThumbnail_t)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
	DwmSetIconicThumbnail_t DwmSetIconicThumbnail;
	typedef HRESULT(WINAPI* DwmSetIconicLivePreviewBitmap_t)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
	DwmSetIconicLivePreviewBitmap_t DwmSetIconicLivePreviewBitmap;
	typedef HRESULT(WINAPI* DwmInvalidateIconicBitmaps_t)(HWND hwnd);
	DwmInvalidateIconicBitmaps_t DwmInvalidateIconicBitmaps;
	typedef HRESULT(WINAPI* DwmEnableBlurBehindWindow_t)(HWND hWnd, const void/*DWM_BLURBEHIND*/ *pBlurBehind);
	DwmEnableBlurBehindWindow_t DwmEnableBlurBehindWindow;

    void correctRect(HWND hwnd, RECT* rect) const;
    void decorrectRect(HWND hwnd, RECT* rect) const;

private:
	HMODULE m_hDwmApi;
	bool    m_bDwmAllowed;
};
