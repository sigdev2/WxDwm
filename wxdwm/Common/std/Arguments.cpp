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

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iterator>
#include <vector>

#if __cplusplus >= 201703L
#include <filesystem>
#else
#include <set>
namespace std
{
    namespace filesystem
    {
        class path
        {
        public:
            path(const std::string& str, const std::set<char>& delimiters = { '\\', '/' })
            {
                char const* pch = str.c_str();
                char const* start = pch;
                for (; *pch; ++pch)
                {
                    if (delimiters.find(*pch) != delimiters.end())
                    {
                        if (start != pch)
                        {
                            std::string str(start, pch);
                            m_result.push_back(str);
                        }
                        else
                        {
                            m_result.push_back("");
                        }
                        start = pch + 1;
                    }
                }
                m_result.push_back(start);
            }
            virtual ~path() {};

            std::string filename() const { return m_result.back(); };

        private:
            std::vector<std::string> m_result;
        };
    }
}
#endif

#include "Arguments.h"

Arguments::Arguments(const std::string& input, const std::string& name, const std::string& ver, const std::string& helpText)
    : m_version(ver), m_name(name), m_text(helpText), m_lastError(eNone), m_parsed(false)
{
    std::istringstream iss(input);
    std::copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter(m_tokens));

    if (!m_tokens.empty())
       m_path = *(m_tokens.begin());

    add({"help", "h", eBool, "Show help", Arguments::ePositional, "", "" });
}

Arguments::Arguments(int argc, char* argv[], const std::string& name, const std::string& ver, const std::string& helpText)
    : m_version(ver), m_name(name), m_text(helpText), m_lastError(eNone), m_parsed(false)
{
    m_tokens.insert(m_tokens.begin(), argv, argv + argc);
    if (!m_tokens.empty())
        m_path = *(m_tokens.begin());
    add({ "help", "h", eBool, "Show help", Arguments::ePositional, "", "" });
}

Arguments Arguments::child(const std::string& input) const
{
    Arguments other(input, m_name, m_version, m_text);
    for (std::unordered_map<std::string, SArgs>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
    {
        SArgs arg = it->second;
        arg.parsed.clear();
        other.add(arg);
    }

    return other;
}

bool Arguments::has(const std::string& key)
{
    if (!parse())
        return false;

    std::unordered_map<std::string, SArgs>::const_iterator it = m_args.find(key);
   
    return it != m_args.end() && !it->second.parsed.empty();
}

std::string Arguments::get(const std::string& key)
{
    if (!parse())
        return std::string();

    std::unordered_map<std::string, SArgs>::const_iterator it = m_args.find(key);
    if (it == m_args.end())
        return std::string();
    return it->second.parsed;
}

std::string Arguments::help(bool useHeader) const
{
    std::string header = m_name + " v" + m_version + "\n\n";

    if (!m_text.empty())
        header += _consoleOut(m_text) + "\n\n";

    size_t max = 0;
    std::string cmd = std::filesystem::path(m_path).filename();
    std::list<std::pair<std::string, std::string> > texts;
    for (std::unordered_map<std::string, SArgs>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
    {
        const SArgs& arg = it->second;
        cmd += " " + (arg.type == eOptional ? "[" + arg.key + "]" : arg.key);
        const std::string argStr = (arg.type == eOptional ? "    [optional] " : "    ") +
                                            arg.key + ", " +
                                            arg.shortkey +
                                            (arg.value == eBool ? "" : " <ARG>");
        const size_t len = argStr.length();
        texts.push_back(std::make_pair(argStr, arg.description));

        if (len > max)
            max = len;
    }

    header += _consoleOut(cmd + "\n\n");

    std::ostringstream outss;
    const std::string defis = " - ";
    const std::string space = "\n" + std::string(max + defis.size(), ' ');
    for (std::list<std::pair<std::string, std::string> >::const_iterator it = texts.begin(); it != texts.end(); ++it)
    {
        const size_t len = it->first.length();
        outss << it->first + std::string(max - len, ' ');
        outss << _consoleOut(defis + it->second, space) + "\n";
    }
    const std::string options = outss.str();
    return (useHeader ? header : "") + (options.empty() ? "" : "Options:\n\n" + options + "\n\n");
}

bool Arguments::parse(bool force)
{
    if (!force && m_parsed)
        return m_lastError == eNone;
    m_parsed = true;

    std::list<std::string> parents;
    std::list<std::string>::const_iterator it = m_tokens.begin();
    ++it;
    for (; it != m_tokens.end(); ++it)
    {
        std::string token = *it;
        
        std::string key = _trimKey(token);
        std::unordered_map<std::string, SArgs>::iterator argIt = m_args.begin();
        for (; argIt != m_args.end(); ++argIt)
        {
            SArgs& a = argIt->second;
            if (key == a.key)
                break;
            if (key == a.shortkey)
            {
                key = a.key;
                break;
            }
        }

        if (argIt == m_args.end())
        {
            _error({ key, key, eBool, "", Arguments::ePositional, key, "" }, eUnknownArg);
            return false;
        }

        SArgs& arg = argIt->second;
        parents.push_back(key);

        if (!arg.parent.empty() && std::find(parents.begin(), parents.end(), arg.parent) == parents.end())
        {
            _error(arg, eNeedParent);
            return false;
        }

        if (arg.value == eBool)
        {
            arg.parsed.swap(key);
        }
        else
        {
            ++it;
            if (it == m_tokens.end())
            {
                _error(arg, eNoArgValue);
                return false;
            }

            arg.parsed = *it;
        }

    }

    _error(SArgs(), eNone);
    return true;
}

void Arguments::_error(const SArgs& arg, const EErrors& code)
{
    std::string text;
    switch (code)
    {
    case eUnknownArg:
        text = "Unknown argument \"" + arg.key + "\"";
        break;
    case eNoArgValue:
        text = "The argument \"" + arg.key + "\" must contain a value as next token";
        break;
    case eNeedParent:
        text = "Argument \"" + arg.key + "\" must be preceded by argument \"" + arg.parent + "\"";
        break;
    default:
    case eNone:
        text.clear();
    }

    m_lastError = code;
    m_lastErrorText.swap(text);
}

std::string Arguments::_trimKey(const std::string& key) const
{
    std::string trimed;
    bool first = true;
    for (std::string::const_iterator ch = key.begin(); ch != key.end(); ++ch)
    {
        if (*ch == eArgPrefix && first)
            continue;

        first = false;
        trimed += *ch;
    }

    return trimed;
}

std::string Arguments::_consoleOut(const std::string& text, const std::string& seporator) const
{
    if (text.empty())
        return std::string();

    std::ostringstream outss;
    char* buffer = new char[eConsoleWidth + 1];
    std::stringstream ss(text);
    bool nofirst = false;
    while (!ss.eof())
    {
        ss.read(buffer, eConsoleWidth);
        buffer[ss.gcount()] = '\0';
        if (nofirst)
            outss << seporator;
        outss << buffer;
        nofirst = true;
    }
    delete[] buffer;
    return outss.str();
}