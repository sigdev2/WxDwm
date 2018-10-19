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

#include <sstream>

#include "Config.h"

std::string __WStrToStr(const std::wstring& ws)
{
    if (ws.empty())
        return std::string();

    const std::locale locale("");
    typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
    const converter_type& converter = std::use_facet<converter_type>(locale);
    std::vector<char> to(ws.length() * (size_t)converter.max_length());
    std::mbstate_t state;
    const wchar_t* from_next;
    char* to_next;
    const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
    if (result == converter_type::ok || result == converter_type::noconv)
    {
        const std::string s(&to[0], to_next);
        return s;
    }

    return std::string();
}

Config::Config(const String& path, const Json& config)
    : m_default(config), m_config(config), m_path(path),
    m_writer(path), m_watched(false), m_modifed(false)
{
    SetOwner(this);
    Bind(wxEVT_FSWATCHER, &Config::OnFileSystemEvent, this);

    load();
}

Config::~Config()
{
    save();
    _stopWatch();
    m_writer.stop();
    m_writer.wait();
}

void Config::save()
{
    if (wxFile::Exists(m_path) && !m_modifed)
        return;

    _stopWatch();
    const std::string json = m_config.dump();
    const wxString path = wxFileName(m_path).GetPathWithSep();
    if (!wxDir::Exists(path))
        wxDir::Make(path);
    m_writer.write(String(json.begin(), json.end()));
    m_modifed = false;
    _startWatch();
}

void Config::load()
{
    if (!wxFile::Exists(m_path))
        return save();

    _stopWatch();
    InFileStream file;
    int counts = AsyncReWriter::eBusyCounts;

    file.open(m_path, std::ios_base::in);
    while (!file.is_open())
    {
        Thread::sleep(AsyncReWriter::eBusyDelay);
        file.open(m_path);
        counts -= 1;

        if (counts <= 0)
        {
            _startWatch();
            return;
        }
    };

    String json((std::istreambuf_iterator<String::value_type>(file)),
                   (std::istreambuf_iterator<String::value_type>()));
    file.close();

    parse(__WStrToStr(json));
}

bool Config::parse(const std::string& json)
{
    _stopWatch();
    std::string errors;
    const Json parsed = Json::parse(json.c_str(), errors);
    if (!errors.empty())
    {
        _startWatch();
        return false;
    }

    if (m_config == parsed)
    {
        _startWatch();
        return false;
    }

    m_config = parsed;
    m_modifed = false;
    _startWatch();
    return true;
}

Json Config::value(const String& key)
{
    return value(_parsePath(key));
}

Json Config::value(const std::vector<String>& keyPath)
{
    Json val = _value(keyPath, &m_config);

    if (val.is_null())
        return _value(keyPath, &m_default);

    return val;
}


void Config::setPath(const String& path)
{
    save();
    _stopWatch();
    m_config = Json();
    m_modifed = false;
    m_path = path;
    load();
}

void Config::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
{
    if (event.GetPath().GetFullName() != wxFileName(m_path).GetFullName())
        return;

    int type = event.GetChangeType();
    if (type == wxFSW_EVENT_DELETE || type == wxFSW_EVENT_RENAME)
    {
        _stopWatch();
        if (m_writer.isEmpty())
        {
            m_modifed = true;
            save();
        }
        _startWatch();
    }
    else if (type == wxFSW_EVENT_MODIFY || type == wxFSW_EVENT_CREATE)
    {
        _stopWatch();
        if (m_writer.isEmpty())
            load();
        _startWatch();
    }
}

void Config::_startWatch()
{
    if (m_watched)
        return;

    if (wxFile::Exists(m_path))
    {
        wxString path = wxFileName(m_path).GetPathWithSep();
        Add(wxFileName(path), wxFSW_EVENT_DELETE | wxFSW_EVENT_RENAME | wxFSW_EVENT_MODIFY | wxFSW_EVENT_CREATE);
        m_watched = true;
    }
}

void Config::_stopWatch()
{
    if (m_watched)
    {
        m_watched = false;
        RemoveAll();
    }
}

Json Config::_value(const std::vector<String>& keyPath, const Json* config)
{
    const Json* start = config;
    std::vector<String>::const_iterator it;
    for (it = keyPath.begin(); it != keyPath.end(); ++it)
    {
        if (!start->is_object())
            break;

        start = &((*start)[__WStrToStr(*it)]);
        if (start->is_null())
            return Json();
    }

    if (it != keyPath.end())
        return Json();

    return *start;
}

std::vector<String> Config::_parsePath(const String& key) const
{
    std::vector<String> parts;
    std::basic_stringstream<String::value_type,
        std::char_traits<String::value_type>,
        std::allocator<String::value_type> > ss(key);
    String temp;
    while (std::getline(ss, temp, _KEYS_SEPORATOR))
        parts.push_back(temp);

    return parts;
}
