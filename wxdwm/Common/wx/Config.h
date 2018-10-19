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

#include <string>
#include <fstream>

#include <wx/fswatcher.h>

#include "../vendors/IJson.h"

#include "../std/AsyncWriter.h"

#ifndef String
#    ifndef UNICODE  
#        define String std::string
#    else
#        define String std::wstring
#    endif // UNICODE
#endif // String

#ifndef InFileStream
#    ifndef UNICODE  
#        define InFileStream std::ifstream
#    else
#        define InFileStream std::wifstream
#    endif // UNICODE
#endif // String

#define _KEYS_SEPORATOR _T('/')

std::string __WStrToStr(const std::wstring& ws);

// todo: debug fs watcher
// todo: Create a class for double-sided locking of read-write events for files

class Config : protected wxFileSystemWatcher
{
public:
    Config(const String& path, const Json& config);
    virtual ~Config();

    void save();
    void load();
    bool parse(const std::string& json);

    // note: makros _KEYS_SEPORATOR as seporator
    Json value(const String& key);
    Json value(const std::vector<String>& keyPath);

    template<typename T>
    Json value(const String& key, const T& val) { return _set(_parsePath(key), &m_config, val); }
    template<typename T>
    Json value(const std::vector<String>& keyPath, const T& val) { return _set(keyPath, &m_config, val); }

    // by types
    bool valueBool(const String& key) { return value(key).bool_value(); };
    int valueInt(const String& key) { return value(key).int_value(); };
    double valueDouble(const String& key) { return value(key).number_value(); };
    String valueString(const String& key) { std::string str = value(key).string_value();
        return String(str.begin(), str.end()); };

    Json defaultConfig() const { return m_default; };
    void setDefult(const Json& def) { m_default = def; };

    void setPath(const String& path);

protected:
    void OnFileSystemEvent(wxFileSystemWatcherEvent& event);

private:
    void _startWatch();
    void _stopWatch();
    Json _value(const std::vector<String>& keyPath, const Json* config);
    std::vector<String> _parsePath(const String& key) const;

    template<typename T>
    Json _set(const std::vector<String>& keyPath, const Json* config, const T& val)
    {
        const Json* parent = config;
        const Json* start = config;
        std::vector<String>::const_iterator it;
        for (it = keyPath.begin(); it != keyPath.end(); ++it)
        {
            if (start->is_object())
                parent = start;
            else
                return Json();

            std::string key = __WStrToStr(*it);
            start = &((*start)[key]);

            if (start->is_null())
            {
                JsonObj& json_obj = const_cast<JsonObj&>(parent->object_items());
                json_obj[key] = JsonObj();
                start = &(json_obj[key]);
                m_modifed = true;
            }
        }

        std::string key = __WStrToStr(*keyPath.rbegin());
        JsonObj& json_obj = const_cast<JsonObj&>(parent->object_items());
        if (json_obj[key] == val)
            return json_obj[key];
        json_obj[key] = val;
        m_modifed = true;
        return json_obj[key];
    }

    Json m_default;
    Json m_config;
    AsyncReWriter m_writer;
    String m_path;
    bool m_watched;
    bool m_modifed;
};

