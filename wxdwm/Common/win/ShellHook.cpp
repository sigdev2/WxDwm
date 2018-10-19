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

#include "ShellHook.h"

ShellHook::ShellHook(HWND hwnd)
	: m_hwnd(hwnd),
	m_fRegisterShellHookWindow(NULL),
	m_fDeregisterShellHookWindow(NULL),
	m_shellhookid(-1)
{
	HMODULE module = GetModuleHandle(L"USER32.DLL");
	if (module != NULL)
	{
		m_fRegisterShellHookWindow = (BOOL(__stdcall *)(HWND))GetProcAddress(module, "RegisterShellHookWindow");
		m_fDeregisterShellHookWindow = (BOOL(__stdcall *)(HWND))GetProcAddress(module, "DeregisterShellHookWindow");

		if (m_fRegisterShellHookWindow != NULL)
		{
			m_fRegisterShellHookWindow(m_hwnd);
			m_shellhookid = RegisterWindowMessage(L"SHELLHOOK");
		}
	}
}

ShellHook::~ShellHook()
{
	if (m_fDeregisterShellHookWindow != NULL)
		m_fDeregisterShellHookWindow(m_hwnd);
}
