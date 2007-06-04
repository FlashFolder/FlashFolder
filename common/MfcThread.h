/* This file is part of FlashFolder. 
 * Copyright (C) 2007 zett42 ( zett42 at users.sourceforge.net ) 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */          
#pragma once

/** \brief Abstract class for creating an MFC worker thread.
*
* This class is basically a wrapper for AfxBeginThread()
*/
class CMfcThread
{
public:
	CMfcThread() :
		m_isCanceled( false ),
		m_isRunning( false ),
		m_pThread( NULL ) {}
	
	/// Free the MFC thread object
	virtual ~CMfcThread()
	{
		delete m_pThread;
	}
	/// Free the MFC thread object
	void Free()
	{
		delete m_pThread;
		m_pThread = NULL;
	}

	/// \brief Automatic casting to the thread handle.
	operator HANDLE() const { return GetHandle(); }

	/// \brief Get thread handle
	HANDLE GetHandle() const { return m_pThread ? m_pThread->m_hThread : NULL; }
	/// \brief Get thread ID
	DWORD GetId() const      { return m_pThread ? m_pThread->m_nThreadID : NULL; }

	/// \brief Set thread priority
	void SetPriority( int prio ) 
	{
		if( m_pThread )
			m_pThread->SetThreadPriority( prio );
	}
	/// \brief Get thread priority
	DWORD GetPriority() const
	{
		return m_pThread ? m_pThread->GetThreadPriority() : THREAD_PRIORITY_NORMAL;
	}

	/// \brief Suspend the thread
	DWORD Suspend()
	{
		if( m_pThread )
			return m_pThread->SuspendThread();
		return 0;
	}
	/// \brief Resume the thread
	DWORD Resume()
	{
		if( m_pThread )
			return m_pThread->ResumeThread();
		return 0;
	}
	/// \brief Wait for the thread to terminate or check if it is running (if timeOut is 0).
	DWORD WaitFor( DWORD timeOutMilliseconds = INFINITE )
	{
		if( m_pThread )
			return ::WaitForSingleObject( *m_pThread, timeOutMilliseconds );
		return WAIT_OBJECT_0;
	}
	/// \brief Get thread exit code
	DWORD GetExitCode() const 
	{ 
		if( ! m_pThread )
			return 0xFFFFFFFF;
		DWORD res;
		if( ::GetExitCodeThread( *m_pThread, &res ) )
			return res;
		return 0xFFFFFFFF;
	}

	/// \brief Return true if the thread function is currently running.
	bool IsRunning() const { return m_isRunning; }

	/// \brief Notify the thread that it should terminate itself now. 
	///
	/// The thread should call IsCanceled() regularly to detect this case.
	/// Override in descendant classes to provide different termination method.
	virtual void SetCanceled()
	{
		m_isCanceled = true;
	}

protected:
	/// \brief Override this function in descendant classes to provide thread functionality.
	virtual UINT ThreadFunc() = 0;

	/// \brief Create the thread
	bool CreateThread( DWORD nPriority = 0, DWORD dwCreateFlags = 0, UINT nStackSize = 0, 
	                   LPSECURITY_ATTRIBUTES lpSec = NULL ) 
	{
		ASSERT( ! m_pThread );
		if( m_pThread )
			return false;

		m_isCanceled = false;

		m_pThread = AfxBeginThread( InternalThreadFunc, this, nPriority, nStackSize, CREATE_SUSPENDED, lpSec );
		if( m_pThread )
		{
			m_pThread->m_bAutoDelete = FALSE;
			if(	! ( dwCreateFlags & CREATE_SUSPENDED ) )
				m_pThread->ResumeThread();
		}
		return m_pThread != NULL;
	}

	/// \brief Check if the default implementation of EndThread() has been called.
	bool IsCanceled() const { return m_isCanceled; }

private:
	static UINT InternalThreadFunc( LPVOID pParam )
	{
		CMfcThread* pThis = static_cast<CMfcThread*>( pParam );
		pThis->m_isRunning = true;
		UINT res = pThis->ThreadFunc();
		pThis->m_isRunning = false;
		return res;
	}

	CWinThread* m_pThread;
	volatile bool m_isCanceled;
	volatile bool m_isRunning;
};
