// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "WinNamedPipe.h"
#include <sstream>

#ifdef _WIN32
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[(size_t)len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

#define THROW_LAST_ERROR(e)									\
{															\
	int error=GetLastError();								\
	std::stringstream err;									\
	err<<e<<", GLE="<<error;								\
	throw std::runtime_error(err.str().data());				\
	err.clear();											\
}

void WinNamedPipe::_closeHandle(HANDLE& pipe)
{
	if (pipe != INVALID_HANDLE_VALUE && pipe != NULL)
	{
		FlushFileBuffers(pipe);
		CloseHandle(pipe); //May throw an exception if a debugger is attached to the process!
	}
	pipe = NULL;
}

WinNamedPipe::WinNamedPipe(HANDLE pipe)
    :_hPipe(pipe),_server_with_client(true)
{
	_server=false;
}

WinNamedPipe::WinNamedPipe(const std::string& name, bool server)
    : _hPipe(NULL),_server_with_client(false),INamedPipe("\\\\.\\pipe\\",name,server)
{
}

void WinNamedPipe::open()
{
	_hPipe = CreateNamedPipe(
		s2ws(_name).data(),             // pipe name 
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,       // read/write access 
		PIPE_TYPE_BYTE |
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUFFER_PIPE_SIZE,                  // output buffer size 
		BUFFER_PIPE_SIZE,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (_hPipe == INVALID_HANDLE_VALUE)
	{
		THROW_LAST_ERROR(_T("CreateNamedPipe failed"));
	}
}

void WinNamedPipe::connect()
{
	for (;;)
	{
		WaitNamedPipe(s2ws(_name).data(), NMPWAIT_WAIT_FOREVER);
		_hPipe = CreateFile(
			s2ws(_name).data(),   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 
							// Break if the pipe handle is valid or error!=232
		if (_hPipe != INVALID_HANDLE_VALUE || GetLastError() != ERROR_PIPE_BUSY)
			break;
	}
	if (_hPipe == INVALID_HANDLE_VALUE)
		THROW_LAST_ERROR(_T("Could not open pipe"));
	// The pipe connected; change to message-read mode. 

	DWORD dwMode = PIPE_TYPE_BYTE;
	BOOL fSuccess = SetNamedPipeHandleState(
		_hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		THROW_LAST_ERROR("SetNamedPipeHandleState failed");
	}
}

WinNamedPipe* WinNamedPipe::WaitForConnection()
{
	if(_server)	 
	{
		DWORD error = 0;
		if (ConnectNamedPipe(_hPipe, NULL) || (error = GetLastError()) == ERROR_PIPE_CONNECTED)
		{
			HANDLE client = _hPipe;
			open();
			return new WinNamedPipe(client);
		}
		else
		{
		}
	}
	else
	{
		throw std::runtime_error("WaitForConnection is not supported on server pipe\n");
	}

	return NULL;
}

void WinNamedPipe::Close()
{
	if(_server||_server_with_client)
		DisconnectNamedPipe(_hPipe); 

	_closeHandle(_hPipe);
}

void  WinNamedPipe::internalReadBytes(void* buf, size_t size)
{
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	// Read from the pipe. 
	fSuccess = ReadFile( 
		_hPipe,        // handle to pipe 
		buf,    // buffer to receive data 
		(DWORD)size, // size of buffer 
		&cbBytesRead, // number of bytes read 
		NULL);        // not overlapped I/O 

	if (!fSuccess || cbBytesRead == 0 || (size_t)cbBytesRead != size)
	{   
		if (GetLastError() == ERROR_BROKEN_PIPE)
		{
			THROW_LAST_ERROR(_T("pipe disconnected")); 
		}
		else
		{
			THROW_LAST_ERROR(_T("read failed")); 
		}
	}

}

void WinNamedPipe::internalFlush()
{
	FlushFileBuffers(_hPipe); 
}

void  WinNamedPipe::internalWriteBytes(const void* buf, size_t size)
{
	DWORD cbWritten;
	BOOL fSuccess = FALSE;

	fSuccess = WriteFile(
		_hPipe,        // handle to pipe 
		buf,     // buffer to write from 
		(DWORD)size, // number of bytes to write 
		&cbWritten,   // number of bytes written 
		NULL);        // not overlapped I/O 
	
	if (!fSuccess || size != (size_t)cbWritten)
	{
		THROW_LAST_ERROR(_T("WriteFile failed")); 
	}
}

WinNamedPipe::~WinNamedPipe(void)
{
	Close();
}

WinNamedPipe*  WinNamedPipe::WaitForConnection(unsigned int timeout)
{
	if(_server)	 
	{
		OVERLAPPED lpOverlapped = {0};
		lpOverlapped.hEvent = CreateEvent(0,1,1,0);
		DWORD error = 0;
		if (ConnectNamedPipe(_hPipe, &lpOverlapped) == 0)
		{
			if (GetLastError() == ERROR_PIPE_CONNECTED)
				if (!SetEvent(lpOverlapped.hEvent))
					THROW_LAST_ERROR("AsyncWaitForConnection failed");

			int result = WaitForSingleObject(lpOverlapped.hEvent, timeout);
			if (WAIT_OBJECT_0 == result)
			{
				HANDLE client = _hPipe;
				open();
				return new WinNamedPipe(client);
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			// ignore connected pipes is ignore, we already worked it
		}
	}
	else
	{
		throw std::runtime_error("WaitForConnection is not supported on client pipe\n");
	}

	return NULL;
}

#endif