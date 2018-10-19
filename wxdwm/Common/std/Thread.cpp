// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/////////////////////////////////////////////////////////////////////////////

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

#include "Thread.h"

/////////////////////////////////////////////////////////////////////////////
// CThread
/////////////////////////////////////////////////////////////////////////////
CThread::CThread() : m_thr(NULL), m_bIsStopped(true), m_bInRun(false)
{
}

/////////////////////////////////////////////////////////////////////////////
CThread::~CThread()
{
    if (m_thr != NULL)
        stop();
}

/////////////////////////////////////////////////////////////////////////////
void CThread::start()
{
    CMutexLocker lc(m_thrMutex);
    m_thr = new std::thread(&CThread::staticRun, this);
    m_bIsStopped = false;
}

/////////////////////////////////////////////////////////////////////////////
void CThread::stop()
{
    CMutexLocker lc(m_thrMutex);
    const bool bThrIsNotNull = (m_thr != NULL);
    lc.unlock();

    lc.lock();
    m_bIsStopped = true;
    lc.unlock();

    if (bThrIsNotNull)
    {
        if (wait())
        {
            lc.lock();
            delete m_thr;
            m_thr = NULL;
            lc.unlock();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
bool CThread::wait()
{
    CMutexLocker lc(m_thrMutex);

    if (m_thr == NULL)
        return false;

    const bool bJoin = m_thr->joinable();
    lc.unlock();

    if (!bJoin)
        return false;

    m_thr->join();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
void CThread::staticRun()
{
    CMutexLocker lc(m_thrMutex);
    m_bInRun = true;
    lc.unlock();

    run();

    lc.lock();
    m_bInRun = false;
}

/////////////////////////////////////////////////////////////////////////////
bool CThread::inRun()
{
    CMutexLocker lc(m_thrMutex);
    return m_bInRun;
}

/////////////////////////////////////////////////////////////////////////////
bool CThread::isStopped()
{
    CMutexLocker lc(m_thrMutex);
    return m_bIsStopped;
}

/////////////////////////////////////////////////////////////////////////////
void CThread::setStopped(bool bStopped)
{
    CMutexLocker lc(m_thrMutex);
    m_bIsStopped = bStopped;
}

/////////////////////////////////////////////////////////////////////////////
// CThreadWhiled
/////////////////////////////////////////////////////////////////////////////
void CThreadWhiled::run()
{
    if (!init())
        return;

    while (!isStopped())
        whileRun();
}

/////////////////////////////////////////////////////////////////////////////