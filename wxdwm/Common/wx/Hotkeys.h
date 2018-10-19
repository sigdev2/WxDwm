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

#include <functional>
#include <unordered_map>

#include "../std/NumberPair.h"

#include "wx/wx.h"

class Hotkeys : public wxEvtHandler
{
public:
	typedef std::function<void (wxFrame*)> TFunc;
	typedef std::unordered_map<NumberPair<int>, std::pair<TFunc, int> > TFuncs;

    struct Hotkey
    {
        Hotkey() : key(0), mod(0) {};
        int key;
        int mod;
    };

	enum Consts
	{
		eIdStart = 0
	};

	Hotkeys(wxFrame* parent);
	~Hotkeys();

    void Add(const wxString& hk, const TFunc& func) { Add(_strToHotkey(hk), func); };
    void Add(const Hotkey& hk, const TFunc& func) { Add(hk.key, hk.mod, func); };
	void Add(int key, int mod, const TFunc& func);

	void OnKey(wxKeyEvent&);
private:
    Hotkeys::Hotkey _strToHotkey(const wxString& str) const;

	TFuncs m_funcs;
	static int m_lastId;
	wxFrame* m_parent;
};

