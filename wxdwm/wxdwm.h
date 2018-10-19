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

#include "MainFrame.h"

// our normal wxApp-derived class, as usual
class App : public wxApp
{
public:

    bool OnInit()
    {
        wxFrame* frame = new MainFrame();
        SetTopWindow(frame);
        frame->Show();
        return true;
    }
};

DECLARE_APP(App);