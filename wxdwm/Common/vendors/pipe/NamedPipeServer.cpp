// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NamedPipeServer.h"

#if (TBB_USE_EXCEPTIONS != 1)
    #define TBB_USE_EXCEPTIONS 1
#endif

void NamedPipeServer::run()
{
	NamedPipe* client;
	_pipe = new NamedPipe(_name, true);
	try
	{
		_pipe->ConnectOrOpen();
	}
	catch(const std::exception&)
	{
	}

	while(1)
	{
		bool localActive = false;
		{
			boost::lock_guard<boost::mutex> lock(_activeMutex);
			localActive = _active;
		}
		if (!localActive)
			break;

		try
		{
			client = _pipe->WaitForConnection(1000);
			if (client != NULL)
				_clients.push(client);
		}
		catch(const std::exception&)
		{
		}
		catch(...)
		{
		}
	}

	delete _pipe;
}

void NamedPipeServer::Start()
{
	{
		boost::lock_guard<boost::mutex> lock(_activeMutex);
		_active = true;
	}
	
	startWorkers();
}

void NamedPipeServer::workerProc()
{
	NamedPipe* client;
	while(1)
	{
		bool localActive = false;
		{
			boost::lock_guard<boost::mutex> lock(_activeMutex);
			localActive = _active;
		}
		if (!localActive)
			break;

		try
		{
			_clients.pop(client);
		}
		catch (...)
		{
			break;
		}
		
		handleClient(boost::shared_ptr<NamedPipe>(client));
	}
}

void NamedPipeServer::startWorkers()
{
	size_t thread_pool_size = boost::thread::hardware_concurrency();
	_threads.reserve(thread_pool_size);
	for (size_t i = 0; i < thread_pool_size; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(&NamedPipeServer::workerProc, this)));
		_threads.push_back(thread);
	}
	boost::shared_ptr<boost::thread> dispatcher(new boost::thread(boost::bind(&NamedPipeServer::run, this)));
	_threads.push_back(dispatcher);
}

void NamedPipeServer::JoinWorkers()
{
	_clients.abort();
	const size_t size = _threads.size();
	if (size > 0)
	{
		for (std::size_t i = 0; i < size; ++i)
			_threads[i]->join();
		for (std::size_t i = 0; i < size; ++i)
			_threads[i].reset();
		_threads.clear();
	}
}

void NamedPipeServer::Stop()
{
	{
		boost::lock_guard<boost::mutex> lock(_activeMutex);
		_active = false;
	}
	
	this->JoinWorkers();
}

NamedPipeServer::~NamedPipeServer(void)
{
	this->Stop();
	while(!_clients.empty())
		if(_clients.try_pop(_pipe))
			delete _pipe;
}
