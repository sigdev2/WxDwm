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

#include <iostream>
#include <fstream>

#ifndef String
#    ifndef UNICODE  
#        define String std::string
#    else
#        define String std::wstring
#    endif // UNICODE
#endif // String

#ifndef OutFileStream
#    ifndef UNICODE  
#        define OutFileStream std::ofstream
#    else
#        define OutFileStream std::wofstream
#    endif // UNICODE
#endif // String

#include "Thread.h"

/////////////////////////////////////////////////////////////////////////////
class AsyncWriter : public CThreadAccumW<String, OutFileStream>
{
    typedef CThreadAccumW<String, OutFileStream> _Base;

public:
    AsyncWriter(const String& path);
	virtual ~AsyncWriter();

    void write(const String& text);

protected:
    virtual bool init();
    virtual void commit();

    virtual void _push(const String& item);
    virtual void _clear() {};

private:
    const String m_path;
};

/////////////////////////////////////////////////////////////////////////////
class AsyncReWriter : public CThreadAccumW<String, String>
{
    typedef CThreadAccumW<String, String> _Base;

public:
    enum EConsts
    {
        eBusyDelay = 300,
        eBusyCounts = 10,
    };

    AsyncReWriter(const String& path);
    virtual ~AsyncReWriter();

    void write(const String& text);

protected:
    virtual bool init() { return true; }
    virtual void commit();

    virtual void _push(const String& item);
    virtual void _clear() {};

private:
    const String m_path;
};

/////////////////////////////////////////////////////////////////////////////