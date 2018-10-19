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

#include "wx/wx.h"

#include "Common/std/Arguments.h"
#include "Common/win/ShellHook.h"
#include "Common/wx/Hotkeys.h"
#include "Common/wx/Config.h"

#include "WindowManager.h"
#include "PipeAPI.h"

#define __MFPtr(o) reinterpret_cast<MainFrame*>(o)
#define __MF_WM_CALL_LAMBDA(c) SHELL_HOOK_LAMBDA(__MFPtr(frame)->manager()->c)
#define __MF_WM_CALL_LAMBDA_IF(a, b) SHELL_HOOK_LAMBDA(if(__MFPtr(frame)->manager()->a) b)
#define __MF_CONFIG_FILE_PATH _T("%HOME%/wxdwm.json")
#define __MF_HK(x) m_config->valueString(_T("Hotkeys/"#x))

class MainFrame : public wxFrame
{
public:
	MainFrame(wxWindow* parent = NULL);
    virtual ~MainFrame();

    Arguments* args() { return m_args; }
	WindowManager* manager() { return m_mgr; }

    void startPipeServer(const std::string& name);

	WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam);

    void OnScreenChanged(wxDisplayChangedEvent& event);

    void saveConfig();
    void loadConfig();
    bool loadJsonConfig(const std::string& text);
    void loadFileConfig(const std::string& path);

    void saveStored();
    void loadStored(bool run = false);
    bool loadJsonStored(const std::string& text, bool run = false);

    std::string commandline(Arguments& args, bool useHeader = true);

private:
	WindowManager* m_mgr;
    PipeAPI* m_srv;
	Hotkeys* m_hk;
	ShellHook* m_hook;
    Config* m_config;
    Arguments* m_args;
};
