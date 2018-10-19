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
#include <functional>

#include <windows.h>

#define WIN_HOOK_LAMBDA(l) [](void* parent, HWND hwnd, LONG id, LONG cid, DWORD dwet, DWORD dwmset) { l; }

class WinEventHook
{
public:
	typedef std::function<void (void*, HWND, LONG, LONG, DWORD, DWORD)> TFunc;
	typedef std::unordered_map<DWORD, TFunc> THooks;
	typedef std::unordered_map<void*, WinEventHook*> TObjects;

	WinEventHook(void* parent = NULL, HWND hwnd = NULL, DWORD start = EVENT_MIN, DWORD end = EVENT_MAX);
	virtual ~WinEventHook();

	void Add(DWORD event, const TFunc& func);

	HWND Hwnd() const { return m_hwnd; };
	THooks::const_iterator Func(DWORD event) const { return m_hooks.find(event); }
	THooks::const_iterator End() const { return m_hooks.end(); }

	static void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
		LONG idObject, LONG idChild,
		DWORD dwEventThread, DWORD dwmsEventTime);

private:
	HWINEVENTHOOK m_hook;
	THooks m_hooks;
	HWND m_hwnd;
	void* m_parent;

	static TObjects* m_sobjects;
};