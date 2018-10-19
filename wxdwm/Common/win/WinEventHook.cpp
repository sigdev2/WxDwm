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

#include "WinEventHook.h"

WinEventHook::TObjects* WinEventHook::m_sobjects = NULL;

WinEventHook::WinEventHook(void* parent, HWND hwnd, DWORD start, DWORD end)
	: m_hwnd(hwnd), m_parent(parent)
{
	m_hook = SetWinEventHook(
		start, end,
		NULL,
		&WinEventHook::HandleWinEvent,
		0, 0,
		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	if (WinEventHook::m_sobjects == NULL)
		WinEventHook::m_sobjects = new TObjects();
	(*WinEventHook::m_sobjects)[m_hook] = this;
}

WinEventHook::~WinEventHook()
{
	UnhookWinEvent(m_hook);
	if (WinEventHook::m_sobjects != NULL)
	{
		WinEventHook::m_sobjects->erase(m_hook);
		if (WinEventHook::m_sobjects->size() <= 0)
		{
			delete WinEventHook::m_sobjects;
			WinEventHook::m_sobjects = NULL;
		}
	}
}

void WinEventHook::Add(DWORD event, const TFunc& func)
{
	m_hooks[event] = func;
}

void WinEventHook::HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
	LONG idObject, LONG idChild,
	DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (WinEventHook::m_sobjects == NULL)
		return;

	TObjects::const_iterator oit = WinEventHook::m_sobjects->find(hook);
	if (oit == WinEventHook::m_sobjects->end())
		return;

	const WinEventHook* self = oit->second;
	const HWND ohwnd = self->Hwnd();
	if (ohwnd != NULL && ohwnd != hwnd)
		return;

	THooks::const_iterator it = self->Func(event);
	if (it != self->End())
		it->second(self->m_parent, hwnd, idObject, idChild, dwEventThread, dwmsEventTime);
}