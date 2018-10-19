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

#include <atomic>

class AtomicFlag
{
public:
    AtomicFlag() { m_b.store(false); };
    virtual ~AtomicFlag() { m_b.store(false); };

    void lock() { m_b.store(false); }
    void unlock() { m_b.store(true); }
    bool locked() const { return !m_b.load(); }

private:
    std::atomic_bool m_b;
};

class AtomicLocker
{
public:
    AtomicLocker(AtomicFlag& flag);
    virtual ~AtomicLocker();

    bool locked() const { return m_flag.locked(); }
    bool before() const { return m_bool; }

private:
    AtomicFlag& m_flag;
    bool m_bool;
};

