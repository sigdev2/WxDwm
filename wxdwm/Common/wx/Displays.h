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
#include "wx/display.h"

#include "../std/numbers.h"

class Displays
{
public:
    enum EScreenMode
    {
        eAll = 0x01FF,// 111111111
        eD1 = 0x0001, // 000000001
        eD2 = 0x0002, // 000000010
        eD3 = 0x0004, // 000000100
        eD4 = 0x0008, // 000001000
        eD5 = 0x0010, // 000010000
        eD6 = 0x0020, // 000100000
        eD7 = 0x0040, // 001000000
        eD8 = 0x0080, // 010000000
        eD9 = 0x0100, // 100000000
    };

    Displays() : m_mode(eAll) { update(); };
    virtual ~Displays() {};

    bool setMode(u32 mode);
    bool toggleMode(u32 mode);
    u32 mode() const { return m_mode; }

    bool update();

    wxRect screen() const { return m_screen; }

    static wxRect MonitorWorkArea(HMONITOR m);
    static wxRect MonitorWorkArea(int m);
    static int count() { return wxDisplay::GetCount(); }

private:
    wxRect _wxClientDisplayRect() const;

    u32 m_mode;
    wxRect m_screen;
};

