/* This file is part of FlashFolder.
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net )
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

#include "MfcThread.h"

//-----------------------------------------------------------------------------------------------
/**
 * Class for downloading files via HTTP / FTP by using URLDownloadToFile() API. 
**/
class CDownloadThread :	public CMfcThread
{
class DownloadCallback;
friend class DownloadCallback;

public:
	/// \brief Parameters for Start()
	struct Params
	{
		CString url;              ///< URL of file to download.
		CString destFilePath;     ///< Local file path where the download should be stored.
		HWND notifyHwnd;          ///< Window to send thread notification messages to.
		UINT notifyMsg;           ///< ID of thread notification message.
		Params() : 
			notifyHwnd( NULL ), notifyMsg( 0 ) {}
	};
	/// \brief Value of WPARAM of thread notification message identifies the thread action.
	enum ThreadNotifyType
	{
		ON_STARTED,  ///< The thread has started.
		ON_ENDED,    ///< The thread is about to end.
		ON_STATUS    ///< This is a download status message. LPARAM contains StatusMsg*.
	};
	/// \brief Data send with thread status message (if WPARAM == ON_STATUS). 
	struct StatusMsg
	{
		ULONG ulProgress;
		ULONG ulProgressMax;
        ULONG ulStatusCode;
		LPCWSTR szStatusText;
		CString friendlyText;
	};

	/// \brief Start the download.
	bool Start( const Params& params );

private:
	UINT ThreadFunc();

	Params m_params;
};
