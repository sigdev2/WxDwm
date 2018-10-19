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

#ifndef __THREAD_H__
#define __THREAD_H__

#include <thread>
#include <mutex>
#include <queue>

/////////////////////////////////////////////////////////////////////////////
class CThread
{
public:
    CThread();
    virtual ~CThread();

    virtual void start();
    virtual void stop();
    virtual bool wait();

    void staticRun();

    virtual bool inRun();

    static void sleep(size_t ms) { std::this_thread::sleep_for(std::chrono::microseconds(ms)); };

protected:
    virtual void run() = 0;
    virtual bool isStopped();
    virtual void setStopped(bool bStopped);

private:
    std::thread* m_thr;
    std::mutex m_thrMutex;
    bool m_bIsStopped;
    bool m_bInRun;
};

/////////////////////////////////////////////////////////////////////////////
class CMutexLocker : public std::unique_lock<std::mutex>
{
public:
    CMutexLocker(std::mutex& mutex) : std::unique_lock<std::mutex>(mutex) {};
    ~CMutexLocker() { if (bool()) unlock(); }
};

/////////////////////////////////////////////////////////////////////////////
class CThreadWhiled : public CThread
{
public:
    CThreadWhiled() : CThread() {};
    virtual ~CThreadWhiled() {};

protected:
    virtual void whileRun() = 0;
    virtual bool init() { return true; };
    virtual void run();
};

/////////////////////////////////////////////////////////////////////////////
template <typename TItem, typename TAccum>
class CThreadAccumW : public CThreadWhiled
{

public:
    enum EConsts
    {
        eQueueBlocksSize = 100,
        eDelay = 100
    };

    CThreadAccumW(size_t nDelay = eDelay, size_t nBlockSize = eQueueBlocksSize)
        : CThreadWhiled(), m_nDelay(nDelay), m_nBlockSize(nBlockSize), m_nPushed(0) {};
    virtual ~CThreadAccumW() { };

    bool isEmpty() { CMutexLocker lc(m_accumMutex); return m_nPushed <= 0; };

    size_t size() { CMutexLocker lc(m_accumMutex); return m_nPushed; };

    void push(const TItem& item)
        { CMutexLocker lc(m_accumMutex); _push(item); ++m_nPushed; };

    void clear()
        { CMutexLocker lc(m_accumMutex); _clear(); m_nPushed = 0; };

protected:
    virtual void _push(const TItem& item) = 0;
    virtual void _clear() = 0;

    virtual bool isStopped() { return CThreadWhiled::isStopped() && isEmpty(); };

    virtual void whileRun()
    {
        const size_t nSize = size();

        if (nSize == 0 || (!CThreadWhiled::isStopped() && nSize < m_nBlockSize))
        {
            // wait queue filling
            sleep(m_nDelay);
            return;
        }

        commitItems();
    };

    virtual void commitItems() { commit(); clear(); };

    virtual void commit() = 0;

protected:
    std::mutex m_accumMutex;
    TAccum     m_accum;
    size_t     m_nPushed;
    size_t     m_nDelay;
    size_t     m_nBlockSize;
};

/////////////////////////////////////////////////////////////////////////////
template <typename TItem, typename TAccum>
class CThreadAccumRW : public CThreadAccumW<TItem, TAccum>
{
    typedef CThreadAccumW<TItem, TAccum> _Base;

public:
    CThreadAccumRW(size_t nDelay = eDelay, size_t nBlockSize = eQueueBlocksSize)
        : _Base(nDelay, nBlockSize) {};
    virtual ~CThreadAccumRW() {};

    TItem pop_front()
        { CMutexLocker lc(m_accumMutex); --m_nPushed; return _pop_front(); };

protected:
    virtual TItem _pop_front() = 0;

    virtual void commitItems()
    {
        size_t n = 0;
        while (!isEmpty())
        {
            addItem(pop_front());
            ++n;

            if (n >= m_nBlockSize)
            {
                commit();
                n = 0;
            }
        }

        if (n > 0)
            commit();
    };

    virtual void addItem(const TItem& item) = 0;
};

/////////////////////////////////////////////////////////////////////////////
template <typename TQueueItem>
class CThreadQueue : public CThreadAccumRW<TQueueItem, std::queue<TQueueItem, std::deque<TQueueItem> > >
{
    typedef CThreadAccumRW<TQueueItem, std::queue<TQueueItem, std::deque<TQueueItem> > > _Base;

public:
    CThreadQueue(size_t nDelay = eDelay, size_t nBlockSize = eQueueBlocksSize)
        : _Base(nDelay, nBlockSize) {};
    virtual ~CThreadQueue() {};

protected:
    typedef std::queue<TQueueItem, std::deque<TQueueItem> > TQueue;

    virtual void _push(const TQueueItem& item) { m_accum.push(item); };
    virtual void _clear() { TQueue().swap(m_accum); };
    virtual TQueueItem _pop_front()
        { TQueueItem item = m_accum.front(); m_accum.pop(); return item; };
};

/////////////////////////////////////////////////////////////////////////////
template <typename TQueueItem>
class CMultiThreadQueue : public CThreadQueue<TQueueItem>
{
    typedef CThreadQueue<TQueueItem> _Base;

public:
    enum EThreadConsts
    {
        eThreadsMax = 8
    };

    CMultiThreadQueue(size_t nDelay = eDelay, size_t nBlockSize = eThreadsMax)
        : _Base(nDelay, nBlockSize)
    {
        m_current.reserve((size_t)eThreadsMax);
    };

    virtual ~CMultiThreadQueue()
    {
        commit();
    };

protected:
    typedef std::vector<std::thread> TVector;
    typedef std::vector<std::thread>::iterator TVectorIter;

    virtual void multicomit(const TQueueItem& item) = 0;

    virtual void addItem(const TQueueItem& item)
    {
        m_current.push_back(std::thread(&CMultiThreadQueue<TQueueItem>::multicomit, this, item));
    };

    virtual void commit()
    {
        if (!m_current.empty())
        {
            for (TVectorIter it = m_current.begin(); it != m_current.end(); ++it)
                it->join();
        }
    };

    TVector m_current;
};

/////////////////////////////////////////////////////////////////////////////
typedef CThread Thread;

/////////////////////////////////////////////////////////////////////////////
#endif // __THREAD_H__