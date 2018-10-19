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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <typename T>
class singleton
{
    singleton(const singleton<T>&);
    void operator=(const singleton<T>&);
    class TCreator : public T{};
protected:
    singleton() {}
public:
	static T& instance() { static TCreator s_inst; return s_inst; }

};

#endif //__SINGLETON_H__