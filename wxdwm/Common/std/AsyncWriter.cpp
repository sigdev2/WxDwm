// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/////////////////////////////////////////////////////////////////////////////

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

#include "AsyncWriter.h"

/////////////////////////////////////////////////////////////////////////////
// AsyncWriter
/////////////////////////////////////////////////////////////////////////////
AsyncWriter::AsyncWriter(const String& path)
    : _Base(), m_path(path)
{
    start();
}

/////////////////////////////////////////////////////////////////////////////
AsyncWriter::~AsyncWriter()
{
    if (!isEmpty())
        commitItems();

    if (m_accum.is_open())
        m_accum.close();

    stop();
}

/////////////////////////////////////////////////////////////////////////////
void AsyncWriter::write(const String& text)
{
    if (text.empty())
        return;
    push(text);
}

/////////////////////////////////////////////////////////////////////////////
bool AsyncWriter::init()
{
    m_accum.open(m_path);
    if (!m_accum.good())
    {
        m_accum.close();
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
void AsyncWriter::commit()
{
    if (m_accum.is_open())
        m_accum.flush();
}

/////////////////////////////////////////////////////////////////////////////
void AsyncWriter::_push(const String& item)
{
    if (m_accum.is_open())
        m_accum.write(item.c_str(), item.size());
}

/////////////////////////////////////////////////////////////////////////////
// AsyncReWriter
/////////////////////////////////////////////////////////////////////////////
AsyncReWriter::AsyncReWriter(const String& path)
    : _Base(100, 1), m_path(path)
{
    start();
}

/////////////////////////////////////////////////////////////////////////////
AsyncReWriter::~AsyncReWriter()
{
    if (!isEmpty())
        commitItems();
    stop();
}

/////////////////////////////////////////////////////////////////////////////
void AsyncReWriter::write(const String& text)
{
    if (text.empty())
        return;
    push(text);
}

/////////////////////////////////////////////////////////////////////////////
void AsyncReWriter::commit()
{
    OutFileStream file;
    int counts = eBusyCounts;

    file.open(m_path, std::ios_base::out | std::ios_base::trunc);
    while (!file.is_open())
    {
        sleep(eBusyDelay);
        file.open(m_path);
        counts -= 1;

        if (counts <= 0)
            return;
    };

    file.write(m_accum.c_str(), m_accum.size());
    file.close();
}

/////////////////////////////////////////////////////////////////////////////
void AsyncReWriter::_push(const String& item)
{
    m_accum = item;
}

/////////////////////////////////////////////////////////////////////////////