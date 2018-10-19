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

#include <wx/stdpaths.h>

#include "wxdwm.h"
#include "Common/vendors/IJson.h"

#include "MainFrame.h"

// todo: pip interface to commands and store/restore windows grids
// todo: layouts- tabbed, accordion, grid
// todo: window to panel
// todo: tile window headers
// todo: indicate frame or icon arrange status
MainFrame::MainFrame(wxWindow* parent)
	: wxFrame(parent, -1, _("wxdwm"), wxDefaultPosition, wxSize(0, 0), wxFRAME_NO_TASKBAR | wxFRAME_TOOL_WINDOW),
	m_hk(NULL),
	m_hook(NULL),
	m_mgr(NULL),
    m_srv(NULL),
    m_config(NULL),
    m_args(NULL)
{
    App& app = wxGetApp();
    m_args = new Arguments(app.argc, app.argv, "WxDWM", "0.1", "WxWidgets based desktop window manager. For <ARG> read in description.");
    m_args->add({ "pipe", "p", Arguments::eVal, "Pipe name for commands", Arguments::eOptional, "", "" });
    m_args->add({ "config", "c", Arguments::eVal, "Use this config 'file'", Arguments::eOptional, "", "" });
    
    // Parse arguments
    const std::string responce = commandline(*m_args);
    if (!responce.empty())
    {
        wxMessageBox(_(responce), _(m_args->appName()), wxICON_INFORMATION);
        Close();
        return;
    }

    // Create members
    m_hk = new Hotkeys(this);
    m_hook = new ShellHook(m_hWnd);
    m_mgr = new WindowManager(m_hWnd);

    // Config file
    wxString homePath(__MF_CONFIG_FILE_PATH);
    const wxString appDir = wxStandardPaths::Get().GetUserDataDir();
    homePath.Replace(_T("%HOME%"), appDir);
    const JsonObj config = JsonObj(
        {
            { "Hotkeys" ,
                JsonObj
                {
                    { "Quit", "ALT+Q" },

                    { "Quit", "ALT+Q" },
                    { "Quit", "ALT+Q" },

                    { "Arrange", "ALT+A" },
                    { "Dearrange", "ALT+Z" },

                    { "ScreenAll", "ALT+SHIFT+0" },
                    { "Screen1", "ALT+SHIFT+1" },
                    { "Screen2", "ALT+SHIFT+2" },
                    { "Screen3", "ALT+SHIFT+3" },
                    { "Screen4", "ALT+SHIFT+4" },
                    { "Screen5", "ALT+SHIFT+5" },
                    { "Screen6", "ALT+SHIFT+6" },
                    { "Screen7", "ALT+SHIFT+7" },
                    { "Screen8", "ALT+SHIFT+8" },
                    { "Screen9", "ALT+SHIFT+9" },

                    { "Reflect", "ALT+/" },
                    { "Decoration", "ALT+*" },

                    { "Orientation", JsonObj
                        {
                            { "Auto", "ALT+=" },
                            { "Vert", "ALT+\\+" },
                            { "Horiz", "ALT+-" }
                        }
                    },

                    { "ReflectAll", "ALT+SHIFT+/" },

                    { "Toggle0", "ALT+0" },
                    { "Toggle1", "ALT+1" },
                    { "Toggle2", "ALT+2" },
                    { "Toggle3", "ALT+3" },
                    { "Toggle4", "ALT+4" },
                    { "Toggle5", "ALT+5" },
                    { "Toggle6", "ALT+6" },
                    { "Toggle7", "ALT+7" },
                    { "Toggle8", "ALT+8" },
                    { "Toggle9", "ALT+9" },

                    { "Store0", "ALT+CTRL+0" },
                    { "Store1", "ALT+CTRL+1" },
                    { "Store2", "ALT+CTRL+2" },
                    { "Store3", "ALT+CTRL+3" },
                    { "Store4", "ALT+CTRL+4" },
                    { "Store5", "ALT+CTRL+5" },
                    { "Store6", "ALT+CTRL+6" },
                    { "Store7", "ALT+CTRL+7" },
                    { "Store8", "ALT+CTRL+8" },
                    { "Store9", "ALT+CTRL+9" }
                }
            }
        });

    if (m_config == NULL)
        m_config = new Config(String(homePath.c_str()), config);
    else
        m_config->setDefult(config);

	// Keys

    m_hk->Add(__MF_HK(Quit), SHELL_HOOK_LAMBDA(frame->Close()));

    // arrange mode
    m_hk->Add(__MF_HK(Arrange), __MF_WM_CALL_LAMBDA(arrange()));
    m_hk->Add(__MF_HK(Dearrange), __MF_WM_CALL_LAMBDA(dearrange()));

    // screen
    m_hk->Add(__MF_HK(ScreenAll), __MF_WM_CALL_LAMBDA(setScreenMode(Displays::eAll)));
    m_hk->Add(__MF_HK(Screen1), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD1)));
    m_hk->Add(__MF_HK(Screen2), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD2)));
    m_hk->Add(__MF_HK(Screen3), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD3)));
    m_hk->Add(__MF_HK(Screen4), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD4)));
    m_hk->Add(__MF_HK(Screen5), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD5)));
    m_hk->Add(__MF_HK(Screen6), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD6)));
    m_hk->Add(__MF_HK(Screen7), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD7)));
    m_hk->Add(__MF_HK(Screen8), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD8)));
    m_hk->Add(__MF_HK(Screen9), __MF_WM_CALL_LAMBDA(toggleScreenMode(Displays::eD9)));

    // single windows
    m_hk->Add(__MF_HK(Reflect), __MF_WM_CALL_LAMBDA(reflect()));
    m_hk->Add(__MF_HK(Decoration), __MF_WM_CALL_LAMBDA(toggleDecoration()));

    m_hk->Add(__MF_HK(Orientation/Auto), __MF_WM_CALL_LAMBDA(orientation(GridItem::eAuto)));
    m_hk->Add(__MF_HK(Orientation/Vert), __MF_WM_CALL_LAMBDA(orientation(GridItem::eV)));
    m_hk->Add(__MF_HK(Orientation/Horiz), __MF_WM_CALL_LAMBDA(orientation(GridItem::eH)));

    // all windows
    m_hk->Add(__MF_HK(ReflectAll), __MF_WM_CALL_LAMBDA(reflect(true)));

    // store api
	m_hk->Add(__MF_HK(Toggle0), __MF_WM_CALL_LAMBDA_IF(toggle(0), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle1), __MF_WM_CALL_LAMBDA_IF(toggle(1), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle2), __MF_WM_CALL_LAMBDA_IF(toggle(2), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle3), __MF_WM_CALL_LAMBDA_IF(toggle(3), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle4), __MF_WM_CALL_LAMBDA_IF(toggle(4), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle5), __MF_WM_CALL_LAMBDA_IF(toggle(5), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle6), __MF_WM_CALL_LAMBDA_IF(toggle(6), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle7), __MF_WM_CALL_LAMBDA_IF(toggle(7), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle8), __MF_WM_CALL_LAMBDA_IF(toggle(8), __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Toggle9), __MF_WM_CALL_LAMBDA_IF(toggle(9), __MFPtr(frame)->saveStored()));

    m_hk->Add(__MF_HK(Store0), __MF_WM_CALL_LAMBDA(store(0); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store1), __MF_WM_CALL_LAMBDA(store(1); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store2), __MF_WM_CALL_LAMBDA(store(2); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store3), __MF_WM_CALL_LAMBDA(store(3); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store4), __MF_WM_CALL_LAMBDA(store(4); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store5), __MF_WM_CALL_LAMBDA(store(5); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store6), __MF_WM_CALL_LAMBDA(store(6); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store7), __MF_WM_CALL_LAMBDA(store(7); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store8), __MF_WM_CALL_LAMBDA(store(8); __MFPtr(frame)->saveStored()));
	m_hk->Add(__MF_HK(Store9), __MF_WM_CALL_LAMBDA(store(9); __MFPtr(frame)->saveStored()));
	/* note: free keys - WXK_BACKSPACE WXK_HOME WXK_END WXK_UP WXK_DOWN WXK_LEFT WXK_RIGHT WXK_PAGEDOWN WXK_PAGEUP WXK_DELETE WXK_INSERT ALTGR*/

    Connect(wxEVT_DISPLAY_CHANGED, wxDisplayChangedEventHandler(MainFrame::OnScreenChanged), NULL, this);
}

MainFrame::~MainFrame()
{
    delete m_mgr;
    delete m_srv;
    delete m_hk;
    delete m_hook;
    delete m_config;
    delete m_args;
}

void MainFrame::startPipeServer(const std::string& name)
{
    delete m_srv;
    if (!name.empty())
    {
        m_srv = new PipeAPI(name, this);
        m_srv->Start();
    }
}

WXLRESULT MainFrame::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (m_hook != NULL)
    {
        if (msg == m_hook->id())
        {
            const HWND hwnd = (HWND)lParam;
            switch (wParam & 0x7fff)
            {
                case HSHELL_WINDOWCREATED:
                    m_mgr->add(hwnd);
                    break;
                case HSHELL_WINDOWDESTROYED:
                    m_mgr->remove(hwnd);
                    break;
                case HSHELL_WINDOWACTIVATED:
                    m_mgr->active(hwnd);
                    break;
            }
        }
        else if (WM_DISPLAYCHANGE == msg)
        {
            // note: param is not client area
            // WORD cxScreen = LOWORD(lParam);
            // WORD cyScreen = HIWORD(lParam);
            m_mgr->updateScreen();
        }
        else if (WM_SETTINGCHANGE == msg && SPI_SETWORKAREA == wParam)
        {
            m_mgr->updateScreen();
        }
    }

	return wxFrame::MSWWindowProc(msg, wParam, lParam);
}

void MainFrame::OnScreenChanged(wxDisplayChangedEvent& event)
{
    if (m_mgr != NULL)
        m_mgr->updateScreen();
}

void MainFrame::saveConfig()
{
    if (m_config != NULL)
        m_config->save();
}

void MainFrame::loadConfig()
{
    if (m_config != NULL)
        m_config->load();
}

bool MainFrame::loadJsonConfig(const std::string& text)
{
    if (m_config == NULL || text.empty())
        return false;

    m_config->save();
    return m_config->parse(text);
}

void MainFrame::loadFileConfig(const std::string& path)
{
    if (m_config == NULL || path.empty())
        return;

    m_config->setPath(String(path.begin(), path.end()));
}

void MainFrame::saveStored()
{
    if (m_config == NULL)
        return;

    std::string errors;
    const Json parsed = Json::parse(__WStrToStr(m_mgr->serialize()).c_str(), errors);
    if (!errors.empty())
        return;

    m_config->value<Json>(_T("ScreenAll"), parsed);
    m_config->save();
}

void MainFrame::loadStored(bool run)
{
    if (m_config == NULL || m_mgr == NULL)
        return;

    Json screens = m_config->value(_T("ScreenAll"));

    if (screens.is_null())
        return;

    if (m_mgr->deserialize(screens, run))
        m_mgr->restoreAll();
}

bool MainFrame::loadJsonStored(const std::string& text, bool run)
{
    if (m_config == NULL || m_mgr == NULL || text.empty())
        return false;

    std::string errors;
    const Json parsed = Json::parse(text.c_str(), errors);
    if (!errors.empty())
        return false;

    if (!m_mgr->deserialize(parsed, run))
        return false;

    m_mgr->restoreAll();
    return true;
}

std::string MainFrame::commandline(Arguments& args, bool useHeader)
{
    if (!m_args->empty())
    {
        if (!m_args->parse())
            return "Error! " + m_args->lastErrorText();

        if (m_args->has("help"))
            return m_args->help(useHeader);

        // Pipe Server
        startPipeServer(m_args->get("pipe"));

        loadFileConfig(m_args->get("config"));

        if (m_mgr != NULL)
        {
            if (m_args->has("toggledecoration"))
                m_mgr->toggleDecoration();

            const std::string orient = m_args->get("orientation");
            if (orient == "h")
                m_mgr->orientation(GridItem::eH);
            else if (orient == "v")
                m_mgr->orientation(GridItem::eV);

            if (m_args->has("reflect"))
                m_mgr->reflect();

            if (m_args->has("reflectall"))
                m_mgr->reflect(true);

            try
            {
                int n = std::stoi(m_args->get("store"));
                m_mgr->store(n);
            }
            catch (...) {}

            try
            {
                int n = std::stoi(m_args->get("restore"));
                m_mgr->restore(n);
            }
            catch (...) {}

            if (m_args->has("restoreall"))
                m_mgr->restoreAll();

            try
            {
                int n = std::stoi(m_args->get("toggle"));
                m_mgr->toggle(n);
            }
            catch (...) { }

            try
            {
                int n = std::stoi(m_args->get("screenmode"));
                if (n > 0 && n < Displays::eAll)
                    m_mgr->toggleScreenMode(n);
                else if (n == 0 || n == Displays::eAll)
                    m_mgr->setScreenMode(Displays::eAll);
                else
                    return std::string("Screen index's combination!");
            }
            catch (...) {}

            if (m_args->has("arrange"))
                m_mgr->arrange();

            if (m_args->has("reflectall"))
                m_mgr->dearrange();

            if (m_args->has("saveconfig"))
                saveConfig();

            if (m_args->has("loadconfig"))
                loadConfig();

            loadJsonConfig(m_args->get("configjson"));

            if (m_args->has("savestored"))
                saveStored();

            if (m_args->has("loadstored"))
                loadStored(m_args->get("loadstored") == "run");

            loadJsonStored(m_args->get("loadstoredjson"), true);
        }
    }

    return std::string();
}