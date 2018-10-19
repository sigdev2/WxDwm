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
#include <unordered_map>
#include <list>
#include <type_traits>

// todo: help for options
// todo: must have options
// note: only ANSI for compatibility
class Arguments
{
public:

    enum ETypes
    {
        eVal,
        eBool
    };

    enum EArgType
    {
        eOptional,
        ePositional
    };

    enum EConsts
    {
        eConsoleWidth = 78,
        eArgPrefix = '-'
    };

    enum EErrors
    {
        eUnknownArg,
        eNoArgValue,
        eNeedParent,
        eNone
    };

    struct SArgs
    {
        std::string key;
        std::string shortkey;
        ETypes      value;
        std::string description;
        EArgType    type;
        std::string parsed;
        std::string parent;
    };

    Arguments(const std::string& input, const std::string& name = "Program", const std::string& ver = "0.1", const std::string& helpText = "");
    Arguments(int argc, char* argv[], const std::string& name = "Program", const std::string& ver = "0.1", const std::string& helpText = "");
    virtual ~Arguments() {};
    
    Arguments& operator()(const SArgs& arg) { return add(arg); };
    Arguments& add(const SArgs& arg) { m_args[arg.key] = arg; return *this; };

    Arguments child(const std::string& input) const;

    bool has(const std::string& key);
    std::string get(const std::string& key);
    bool empty() const { return m_tokens.size() <= 1; };

    std::string help(bool useHeader = true) const;

    EErrors lastError() const { return m_lastError; };
    std::string lastErrorText() const { return m_lastErrorText; };
    std::string version() const { return m_version; };
    std::string appName() const { return m_name; };
    std::string appDescription() const { return m_text; };
    std::string appPath() const { return m_path; }

    bool parse(bool force = false);

private:
    std::unordered_map<std::string, SArgs> m_args;
    std::list<std::string> m_tokens;
    EErrors m_lastError;
    std::string m_lastErrorText;
    std::string m_name;
    std::string m_text;
    std::string m_version;
    bool m_parsed;
    std::string m_path;

    void _error(const SArgs& arg, const EErrors& code);
    std::string _trimKey(const std::string& key) const;
    std::string _consoleOut(const std::string& text, const std::string& seporator = "\n") const;
};