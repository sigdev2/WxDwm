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

#include "MainFrame.h"

#include "PipeAPI.h"

PipeAPI::PipeAPI(const std::string& name, MainFrame* parent)
    : NamedPipeServer(name + _PIPE_PREFIX), m_parent(parent)
{
}

void PipeAPI::handleClient(const boost::shared_ptr<NamedPipe>& client)
{
    try
    {
        size_t size;
        client->ReadBytes(&size, sizeof(size));
        if (size > 0)
        {
            char* message = new char[size];
            client->ReadBytes(message, size);
            
            const std::string msg(message, size);
            Arguments args = m_parent->args()->child(msg);
            args.add({ "orientation", "o", Arguments::eVal, "'h' or 'v' for vertical or horizontal orientation current window", Arguments::eOptional, "", "" });
            args.add({ "toggledecoration", "td", Arguments::eBool, "Toggle decoration of current window", Arguments::eOptional, "", "" });
            args.add({ "screenmode", "sm", Arguments::eVal, "Toggle screen using arrange on multipy sreens by number 'n' of binary or combination of screen indexes", Arguments::eOptional, "", "" });
            args.add({ "toggle", "t", Arguments::eVal, "Toggle hide/show of stored windows set by 'index'", Arguments::eOptional, "", "" });
            args.add({ "store", "s", Arguments::eVal, "Store windows set by index", Arguments::eOptional, "", "" });
            args.add({ "restore", "r", Arguments::eVal, "Restore windows set by index", Arguments::eOptional, "", "" });
            args.add({ "restoreall", "ra", Arguments::eBool, "Restore all windows set by index", Arguments::eOptional, "", "" });
            args.add({ "reflect", "rf", Arguments::eBool, "Reflect current selected grid sector", Arguments::eOptional, "", "" });
            args.add({ "reflectall", "rfa", Arguments::eBool, "Reflect current all grid", Arguments::eOptional, "", "" });
            args.add({ "arrange", "a", Arguments::eBool, "Arrange", Arguments::eOptional, "", "" });
            args.add({ "dearrange", "d", Arguments::eBool, "Dearrange", Arguments::eOptional, "", "" });
            args.add({ "saveconfig", "sc", Arguments::eBool, "Save config", Arguments::eOptional, "", "" });
            args.add({ "loadconfig", "lc", Arguments::eBool, "Load cnfig", Arguments::eOptional, "", "" });
            args.add({ "configjson", "cj", Arguments::eVal, "Use thie config 'json'", Arguments::eOptional, "", "" });
            args.add({ "savestored", "ss", Arguments::eBool, "Save stored windows sets", Arguments::eOptional, "", "" });
            args.add({ "loadstored", "ls", Arguments::eVal, "Load stored windows sets, with 'run' or 'no'", Arguments::eOptional, "", "" });
            args.add({ "loadstoredjson", "lj", Arguments::eVal, "Load stored windows sets from 'json', its always run new instance of windows", Arguments::eOptional, "", "" });

            std::string responce = m_parent->commandline(args, false);
            if (responce.empty())
                responce = "true";

            size = responce.size();
            client->WriteBytes(&size, sizeof(size));
            client->WriteBytes(responce.data(), size);

            delete[] message;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception!:" << e.what() << "\n";
    }

    client->Close();
}
