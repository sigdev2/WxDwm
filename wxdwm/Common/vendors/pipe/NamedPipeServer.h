#pragma once

#include "NamedPipe.h"
#include <vector>
#include <boost/thread.hpp>
#include <tbb/concurrent_queue.h>

/*!
* \brief
* This class contains the main functions to make an multithreaded NamedPipe server
* 
* Inherit from it and override the handleClient(INamedPipe* client) function to make your own server
*/
class NamedPipeServer
{
protected:
	///the server pipe name 
	std::string _name;
	/// the pointer to server pipe
	NamedPipe* _pipe;
	///activity. change to false in order to make all threads stop
	bool volatile _active;
	///this queue stores the pointers to connected clients
	tbb::concurrent_bounded_queue<NamedPipe*> _clients;
	///this vector stores all threads serving clients
	std::vector<boost::shared_ptr<boost::thread> > _threads;

	boost::mutex _activeMutex;

	/*!
	* \brief
	* Starts the main loop (waits for new clients and puts them into queue)
	*/
	void run();
	/*!
	* \brief
	* Starts all worker theads
	*/
	void startWorkers();
	/*!
	* \brief
	* Pops the client from queue and passes it to handleClient()
	*/
	void workerProc();
	/*!
	* \brief
	* This function must be overriden. It is intended for handling data from clients;
	*/
	virtual void handleClient(const boost::shared_ptr<NamedPipe>& client) { };
public:
	/*!
	* \brief
	* Starts the server and all worker theads. You must create a new thread for this func.
	*/
	virtual void Start();
	/*!
	* \brief
	* Stops all worker threads and main thread - run()
	*/
	virtual void Stop();
	/*!
	* \brief
	* Joins all worker theads
	*/
	void JoinWorkers();
	/*!
	* \brief
	* Write brief comment for NamedPipeServer here.
	* 
	* \param name
	* The name for server pipe
	*/
	NamedPipeServer(const std::string& name) : _name(name), _pipe(NULL), _active(false) {}
	virtual ~NamedPipeServer();
};