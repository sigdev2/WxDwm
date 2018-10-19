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

#include <wx/tokenzr.h>

#include "Hotkeys.h"

int Hotkeys::m_lastId = Hotkeys::eIdStart;

Hotkeys::Hotkeys(wxFrame* parent)
	: m_parent(parent)
{
}

Hotkeys::~Hotkeys()
{
	for (TFuncs::iterator it = m_funcs.begin(); it != m_funcs.end(); ++it)
	{
		m_parent->Disconnect(it->second.second, wxEVT_HOTKEY, wxKeyEventHandler(Hotkeys::OnKey), NULL, this);
		m_parent->UnregisterHotKey(it->second.second);
	}
}

void Hotkeys::Add(int key, int mod, const TFunc& func)
{
	m_parent->RegisterHotKey(m_lastId, mod, key);
	m_funcs[std::make_pair(key, mod)] = std::make_pair(func, m_lastId);
	m_parent->Connect(m_lastId, wxEVT_HOTKEY, wxKeyEventHandler(Hotkeys::OnKey), NULL, this);
	++m_lastId;
}

void Hotkeys::OnKey(wxKeyEvent& event)
{
	const int key = (int)event.GetRawKeyCode();
	const int mod = (int)event.GetModifiers();
	TFuncs::iterator it = m_funcs.find(std::make_pair(key, mod));
	if (it != m_funcs.end())
	{
		it->second.first(m_parent);
		event.Skip();
	}
}

Hotkeys::Hotkey Hotkeys::_strToHotkey(const wxString& str) const
{
    Hotkeys::Hotkey h;
    wxStringTokenizer tokenizer(str, _T("+"));
    while (tokenizer.HasMoreTokens())
    {
        const wxString token = tokenizer.GetNextToken();

        if (token == _T("ALT")) // left ALT
            h.mod |= wxMOD_ALT;
        else if (token == _T("CTRL") || token == _T("CONTROL") || token == _T("CMD"))
            h.mod |= wxMOD_CONTROL;
        else if (token == _T("RAW_CONTROL"))
            h.mod |= wxMOD_RAW_CONTROL;
        else if (token == _T("WIN") || token == _T("META"))
            h.mod |= wxMOD_WIN;
        else if (token == _T("SHIFT"))
            h.mod |= wxMOD_SHIFT;
        else if (token == _T("ALTGR")) // right ALT
            h.mod |= wxMOD_ALTGR;
        else if (token == "\\") // note: next + is token, then insert his
            h.key = (TCHAR)'+';
        else if (token != _T(" ") && token != _T("\t") && token != _T("\n") && token != _T("\r"))
            h.key = token.at(0);
    }

    return h;
}