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

#include <list>

template<typename T>
class LeazyNumber
{
public:
    LeazyNumber(T base = 0) : m_base(base) {};
    ~LeazyNumber() {};

    operator T() const
    {
        T val = m_base;
        for each (SOperation var in m_ops)
        {
            switch (var.op)
            {
            case eMult: val = static_cast<T>(val * *(var.value)); break;
            case eDiv: val = static_cast<T>(val / *(var.value)); break;
            case ePlus: val = static_cast<T>(val + *(var.value)); break;
            case eMinus: val = static_cast<T>(val - *(var.value)); break;
            default:
                break;
            }
        }

        return val;
    }

    T write()
    {
        for each (SOperation var in m_ops)
        {
            switch (var.op)
            {
            case eMult: m_base = static_cast<T>(m_base * *(var.value)); break;
            case eDiv: m_base = static_cast<T>(m_base / *(var.value)); break;
            case ePlus: m_base = static_cast<T>(m_base + *(var.value)); break;
            case eMinus: m_base = static_cast<T>(m_base - *(var.value)); break;
            default:
                break;
            }
        }

        m_ops.clear();

        return m_base;
    }

    LeazyNumber& mult(double* val) { m_ops.push_back({ val, eMult }); return *this; };
    LeazyNumber& div(double* val) { m_ops.push_back({ val, eDiv }); return *this; };
    LeazyNumber& plus(double* val) { m_ops.push_back({ val, ePlus }); return *this; };
    LeazyNumber& minus(double* val) { m_ops.push_back({ val, eMinus }); return *this; };

private:
    enum EOperator
    {
        eMult,
        eDiv,
        ePlus,
        eMinus
    };

    struct SOperation
    {
        double* value;
        EOperator op;
    };

    T m_base;
    std::list<SOperation> m_ops;
};