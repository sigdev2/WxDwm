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

#include <utility>

#include <string>

#include "numbers.h"

template<typename T>
class NumberPair : public std::pair<T, T>
{
public:
    NumberPair() : std::pair<T, T>() {};
    NumberPair(const std::pair<T, T>& pair) : std::pair<T, T>(pair) {};
    NumberPair(const NumberPair& pair) : std::pair<T, T>(pair) {};
    virtual ~NumberPair() {};

	bool operator< (const NumberPair<T>& pair) const
    {
        std::string f = std::to_string(this->first);
        f.append(std::to_string(this->second));

        std::string s = std::to_string(pair.first);
        s.append(std::to_string(pair.second));

        return s < f;
    }

	bool operator==(const NumberPair<T>& pair) const
    {
        return pair.first == this->first && pair.second == this->second;
    }
};

namespace std
{
	template <>
	struct hash<NumberPair<u32> >
	{
		std::size_t operator()(const NumberPair<u32>& pair) const
		{
			std::string second = std::to_string(pair.first);
			second.append(std::to_string(pair.second));
			return std::hash<std::string>()(second);
		}
	};

    template <>
    struct hash<NumberPair<int> >
    {
        std::size_t operator()(const NumberPair<int>& pair) const
        {
            std::string second = std::to_string(pair.first);
            second.append(std::to_string(pair.second));
            return std::hash<std::string>()(second);
        }
    };
}