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

#include <functional>
#include <unordered_map>

template<typename T>
class Cache
{
public:
    typedef std::function<T()> TFunc;
 
    Cache(TFunc f) : m_f(f), m_ret(NULL) {};
    virtual ~Cache() { delete m_ret; };

    T operator()() { if (m_ret == NULL) m_ret = new T(m_f()); return *m_ret; };
    void reset() { delete m_ret; m_ret = NULL; };

private:
    TFunc m_f;
    T* m_ret;
};

template<typename A, typename T>
class CacheMap
{
public:
    typedef std::function<T(A)> TFunc;

    CacheMap(TFunc f) : m_f(f) {};
    virtual ~CacheMap() { };

    T operator()(const A& arg)
    {
        std::unordered_map<A, T>::const_iterator it = m_ret.find(arg);
        if (it != m_ret.end())
            return it->second;

        T val = m_f(arg);
        m_ret[arg] = val;
        return val;
    };

    void reset() { m_ret.clear(); };

private:
    TFunc m_f;
    std::unordered_map<A, T> m_ret;
};