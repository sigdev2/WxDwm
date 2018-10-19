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

#include <windows.h>
#include <VersionHelpers.h>

#include "DwmHelper.h"

CDwmHelper::CDwmHelper()
	: m_bDwmAllowed(false),
	m_hDwmApi(NULL),
	DwmIsCompositionEnabled(NULL),
	DwmSetWindowAttribute(NULL),
	DwmGetWindowAttribute(NULL),
	DwmExtendFrameIntoClientArea(NULL),
	DwmDefWindowProc(NULL),
	DwmSetIconicThumbnail(NULL),
	DwmSetIconicLivePreviewBitmap(NULL),
	DwmInvalidateIconicBitmaps(NULL),
	DwmEnableBlurBehindWindow(NULL)
{
	/*OSVERSIONINFO info;
	ZeroMemory(&info, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&info))
		return;

	if (info.dwMajorVersion >= 6)*/
	if (IsWindows8OrGreater())
	{
		m_hDwmApi = GetModuleHandle(L"dwmapi.dll");
		if (m_hDwmApi != NULL)
		{
			DwmIsCompositionEnabled = (DwmIsCompositionEnabled_t)GetProcAddress(m_hDwmApi, "DwmIsCompositionEnabled");
			DwmSetWindowAttribute = (DwmSetWindowAttribute_t)GetProcAddress(m_hDwmApi, "DwmSetWindowAttribute");
			DwmGetWindowAttribute = (DwmGetWindowAttribute_t)GetProcAddress(m_hDwmApi, "DwmGetWindowAttribute");
			DwmExtendFrameIntoClientArea = (DwmExtendFrameIntoClientArea_t)GetProcAddress(m_hDwmApi, "DwmExtendFrameIntoClientArea");
			DwmDefWindowProc = (DwmDefWindowProc_t)GetProcAddress(m_hDwmApi, "DwmDefWindowProc");
			DwmSetIconicThumbnail = (DwmSetIconicThumbnail_t)GetProcAddress(m_hDwmApi, "DwmSetIconicThumbnail");
			DwmSetIconicLivePreviewBitmap = (DwmSetIconicLivePreviewBitmap_t)GetProcAddress(m_hDwmApi, "DwmSetIconicLivePreviewBitmap");
			DwmInvalidateIconicBitmaps = (DwmInvalidateIconicBitmaps_t)GetProcAddress(m_hDwmApi, "DwmInvalidateIconicBitmaps");
			DwmEnableBlurBehindWindow = (DwmEnableBlurBehindWindow_t)GetProcAddress(m_hDwmApi, "DwmEnableBlurBehindWindow");

			m_bDwmAllowed = (DwmIsCompositionEnabled != NULL)
				&& (DwmGetWindowAttribute != NULL) && (DwmSetWindowAttribute != NULL)
				&& (DwmExtendFrameIntoClientArea != NULL)
				&& (DwmDefWindowProc != NULL);
		}
	}
}

CDwmHelper::~CDwmHelper()
{
}

bool CDwmHelper::IsDwm() const
{
	if (!m_bDwmAllowed)
		return false;
	BOOL composition_enabled = FALSE;
	return (DwmIsCompositionEnabled(&composition_enabled) == S_OK) && composition_enabled;
}

bool CDwmHelper::IsDwmAllowed() const
{
	return m_bDwmAllowed;
}

void CDwmHelper::correctRect(HWND hwnd, RECT* rect) const
{
    if (!IsDwm())
        return;

    RECT frame;
    RECT rc;
    ::GetWindowRect(hwnd, &rc);
    DwmGetWindowAttribute(hwnd, CDwmHelper::DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(RECT));

    RECT border;
    border.left = frame.left - rc.left;
    border.top = frame.top - rc.top;
    border.right = rc.right - frame.right;
    border.bottom = rc.bottom - frame.bottom;

    rect->left -= border.left;
    rect->top -= border.top;
    rect->right += border.right;
    rect->bottom += border.bottom;
}

void CDwmHelper::decorrectRect(HWND hwnd, RECT* rect) const
{
    if (!IsDwm())
        return;

    RECT frame;
    RECT rc;
    ::GetWindowRect(hwnd, &rc);
    DwmGetWindowAttribute(hwnd, CDwmHelper::DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(RECT));

    RECT border;
    border.left = frame.left - rc.left;
    border.top = frame.top - rc.top;
    border.right = rc.right - frame.right;
    border.bottom = rc.bottom - frame.bottom;

    rect->left += border.left;
    rect->top += border.top;
    rect->right -= border.right;
    rect->bottom -= border.bottom;
}