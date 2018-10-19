#pragma once
#ifdef _WIN32
#include "INamedPipe.h"
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>

#define BUFFER_PIPE_SIZE 5120

	class WinNamedPipe :
		public INamedPipe
	{
		HANDLE _hPipe;
		bool _server_with_client;
		void _closeHandle(HANDLE& pipe);
	protected:
		void connect();
		void open();
		void internalReadBytes(void* buf,size_t size);
		void internalWriteBytes(const void* buf,size_t size);
		void internalFlush();
	public:
		void Close();
		WinNamedPipe* WaitForConnection();
		WinNamedPipe* WaitForConnection(unsigned int timeout);
		WinNamedPipe(HANDLE pipe);
		WinNamedPipe(const std::string& name, bool server);
		~WinNamedPipe();
	};
#endif
