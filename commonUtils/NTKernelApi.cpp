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
 *
 */

#include "stdafx.h"

#pragma warning(disable:4786)   // disable STL-template-related warnings

#include "NtKernelApi.h"
#include <tchar.h>

#include <map>
using namespace std;

namespace NT
{

//-------------------------------------------------------------------------------------------------
// some helpers for automatic dynamic linking to the DLL functions follow

HMODULE g_hNtDll = ::GetModuleHandle( _T("ntdll.dll") );

// function pointer definitions
PNtQuerySystemInformation g_pNtQuerySystemInformation = NULL;
PNtQueryObject g_pNtQueryObject = NULL;
PNtOpenDirectoryObject g_pNtOpenDirectoryObject = NULL;
PNtQueryDirectoryObject g_pNtQueryDirectoryObject = NULL;
PNtClose g_pNtClose = NULL;
PNtCreateFile g_pNtCreateFile = NULL;
PNtOpenFile g_pNtOpenFile = NULL;
PNtQueryInformationFile g_pNtQueryInformationFile = NULL;
PNtSetInformationFile g_pNtSetInformationFile = NULL;
PNtQueryAttributesFile g_pNtQueryAttributesFile = NULL;

//--------------------------------------------------------------------------------------------

// increment a pointer by byteCount bytes, independend of the pointer type
template<class T> inline T* IncBytePtr( const T* p, size_t byteCount )
{
    return ( reinterpret_cast<T*>( reinterpret_cast<DWORD>( p ) + byteCount ) );
}

template<typename T> T AlignDWord( T value ) { return (value + 3) & ~3; }

//--------------------------------------------------------------------------------------------

// CTypeNameCompare, CTypeNameMap are used to map OS-dependend type-IDs to logical object IDs
//    if we have an OS for which we don't know the excact type-ID values.
struct CTypeNameCompare 
{
    bool operator()( const WCHAR* s1, const WCHAR* s2 ) const
        { return wcscmp( s1, s2 ) < 0; };
};

typedef map< WCHAR*, BYTE, CTypeNameCompare > CTypeNameMap;

//--------------------------------------------------------------------------------------------

// the one-and-only instance of this class:
BYTE CNtObjTypeMap::s_typemap[_OT_MaxType];
bool CNtObjTypeMap::s_bInitialised = false;

void CNtObjTypeMap::Init()
{
    OSVERSIONINFO ovi = { sizeof(ovi) };
    ::GetVersionEx( &ovi );
    DWORD os_ver = (ovi.dwMajorVersion << 8) | ovi.dwMinorVersion;

    s_typemap[OT_Unknown] = 0;
    if( os_ver < 0x0500 )    // NT 4 ?
    {
        s_typemap[OT_Type] = OT4_Type;
        s_typemap[OT_Directory] = OT4_Directory;
        s_typemap[OT_SymbolicLink] = OT4_SymbolicLink;
        s_typemap[OT_Token] = OT4_Token;
        s_typemap[OT_Process] = OT4_Process;
        s_typemap[OT_Thread] = OT4_Thread;
        s_typemap[OT_Job] = 0;     
        s_typemap[OT_DebugObject] = 0;    
        s_typemap[OT_Event] = OT4_Event;
        s_typemap[OT_EventPair] = OT4_EventPair;
        s_typemap[OT_Mutant] = OT4_Mutant;
        s_typemap[OT_Callback] = 0;     
        s_typemap[OT_Semaphore] = OT4_Semaphore;
        s_typemap[OT_Timer] = OT4_Timer;
        s_typemap[OT_Profile] = OT4_Profile;
        s_typemap[OT_KeyedEvent] = 0; 
        s_typemap[OT_WindowStation] = OT4_WindowStation;
        s_typemap[OT_Desktop] = OT4_Desktop;
        s_typemap[OT_Section] = OT4_Section;
        s_typemap[OT_Key] = OT4_Key;
        s_typemap[OT_Port] = OT4_Port;
        s_typemap[OT_WaitablePort] = 0;    
        s_typemap[OT_Adapter] = OT4_Adapter;
        s_typemap[OT_Controller] = OT4_Controller;
        s_typemap[OT_Device] = OT4_Device;
        s_typemap[OT_Driver] = OT4_Driver;
        s_typemap[OT_IO_Completion] = OT4_IO_Completion;
        s_typemap[OT_File] = OT4_File;
        s_typemap[OT_WmiGuid] = 0;
    }
    else if( os_ver == 0x0500 )   // Win2k ?
    {
        s_typemap[OT_Type] = OT2K_Type;
        s_typemap[OT_Directory] = OT2K_Directory;
        s_typemap[OT_SymbolicLink] = OT2K_SymbolicLink;
        s_typemap[OT_Token] = OT2K_Token;
        s_typemap[OT_Process] = OT2K_Process;
        s_typemap[OT_Thread] = OT2K_Thread;
        s_typemap[OT_Job] = OT2K_Job;     
        s_typemap[OT_DebugObject] = 0;    
        s_typemap[OT_Event] = OT2K_Event;
        s_typemap[OT_EventPair] = OT2K_EventPair;
        s_typemap[OT_Mutant] = OT2K_Mutant;
        s_typemap[OT_Callback] = OT2K_Callback;     
        s_typemap[OT_Semaphore] = OT2K_Semaphore;
        s_typemap[OT_Timer] = OT2K_Timer;
        s_typemap[OT_Profile] = OT2K_Profile;
        s_typemap[OT_KeyedEvent] = 0; 
        s_typemap[OT_WindowStation] = OT2K_WindowStation;
        s_typemap[OT_Desktop] = OT2K_Desktop;
        s_typemap[OT_Section] = OT2K_Section;
        s_typemap[OT_Key] = OT2K_Key;
        s_typemap[OT_Port] = OT2K_Port;
        s_typemap[OT_WaitablePort] = OT2K_WaitablePort;    
        s_typemap[OT_Adapter] = OT2K_Adapter;
        s_typemap[OT_Controller] = OT2K_Controller;
        s_typemap[OT_Device] = OT2K_Device;
        s_typemap[OT_Driver] = OT2K_Driver;
        s_typemap[OT_IO_Completion] = OT2K_IO_Completion;
        s_typemap[OT_File] = OT2K_File;
        s_typemap[OT_WmiGuid] = 0;
    }
    else if( os_ver == 0x0501 )   // WinXP ?
    {
        s_typemap[OT_Type] = OTXP_Type;
        s_typemap[OT_Directory] = OTXP_Directory;
        s_typemap[OT_SymbolicLink] = OTXP_SymbolicLink;
        s_typemap[OT_Token] = OTXP_Token;
        s_typemap[OT_Process] = OTXP_Process;
        s_typemap[OT_Thread] = OTXP_Thread;
        s_typemap[OT_Job] = OTXP_Job;     
        s_typemap[OT_DebugObject] = OTXP_DebugObject;    
        s_typemap[OT_Event] = OTXP_Event;
        s_typemap[OT_EventPair] = OTXP_EventPair;
        s_typemap[OT_Mutant] = OTXP_Mutant;
        s_typemap[OT_Callback] = OTXP_Callback;     
        s_typemap[OT_Semaphore] = OTXP_Semaphore;
        s_typemap[OT_Timer] = OTXP_Timer;
        s_typemap[OT_Profile] = OTXP_Profile;
        s_typemap[OT_KeyedEvent] = OTXP_KeyedEvent; 
        s_typemap[OT_WindowStation] = OTXP_WindowStation;
        s_typemap[OT_Desktop] = OTXP_Desktop;
        s_typemap[OT_Section] = OTXP_Section;
        s_typemap[OT_Key] = OTXP_Key;
        s_typemap[OT_Port] = OTXP_Port;
        s_typemap[OT_WaitablePort] = OTXP_WaitablePort;    
        s_typemap[OT_Adapter] = OTXP_Adapter;
        s_typemap[OT_Controller] = OTXP_Controller;
        s_typemap[OT_Device] = OTXP_Device;
        s_typemap[OT_Driver] = OTXP_Driver;
        s_typemap[OT_IO_Completion] = OTXP_IO_Completion;
        s_typemap[OT_File] = OTXP_File;   
        s_typemap[OT_WmiGuid] = OTXP_WmiGuid;
    }
    else    // Windows Server 2003 / Longhorn / ??      
    {
        // Since I don't know the excact type-IDs for those OS, I get the values by calling 
        // NtQueryObject() with ObjectAllTypesInformation. 
        // I noticed that only in XP NtQueryObject() retrieves really ALL object types whereas 
        //   this wasn't the case with NT4/2k were some of them (e.g. "File") are missing. 
        // It is assumed, that newer OS versions will show the XP-behaviour.
        // I have tested the code successfully under XP.

        memset( s_typemap, 0, sizeof(s_typemap) );            

        CTypedBuf<OBJECT_ALL_TYPES_INFORMATION> buf( 4096 );
       
        DWORD resLen = 0;
        NTSTATUS res;
        while( ( res = D_NtQueryObject( NULL, ObjectAllTypesInformation, buf.Get(), buf.GetSize(), 
                                        &resLen ) == STATUS_INFO_LENGTH_MISMATCH ) )
            buf.Resize( resLen );

        if( res == STATUS_SUCCESS )
        {
            CTypeNameMap typenameMap;
            typenameMap.insert( CTypeNameMap::value_type( L"Type", OT_Type ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Directory", OT_Directory ) );
            typenameMap.insert( CTypeNameMap::value_type( L"SymbolicLink", OT_SymbolicLink ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Token", OT_Token ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Process", OT_Process ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Thread", OT_Thread ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Job", OT_Job ) );
            typenameMap.insert( CTypeNameMap::value_type( L"DebugObject", OT_DebugObject ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Event", OT_Event ) );
            typenameMap.insert( CTypeNameMap::value_type( L"EventPair", OT_EventPair ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Mutant", OT_Mutant ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Callback", OT_Callback ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Semaphore", OT_Semaphore ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Timer", OT_Timer ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Profile", OT_Profile ) );
            typenameMap.insert( CTypeNameMap::value_type( L"KeyedEvent", OT_KeyedEvent ) );
            typenameMap.insert( CTypeNameMap::value_type( L"WindowStation", OT_WindowStation ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Desktop", OT_Desktop ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Section", OT_Section ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Key", OT_Key ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Port", OT_Port ) );
            typenameMap.insert( CTypeNameMap::value_type( L"WaitablePort", OT_WaitablePort ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Adapter", OT_Adapter ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Controller", OT_Controller ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Device", OT_Device ) );
            typenameMap.insert( CTypeNameMap::value_type( L"Driver", OT_Driver ) );
            typenameMap.insert( CTypeNameMap::value_type( L"IO_Completion", OT_IO_Completion ) );
            typenameMap.insert( CTypeNameMap::value_type( L"File", OT_File ) );
            typenameMap.insert( CTypeNameMap::value_type( L"WmiGuid", OT_WmiGuid ) );
        
            // map the system-dependend object type IDs to the logical object IDs
            // by using the type name which is system-independend

            OBJECT_TYPE_INFORMATION* pt = &buf.Get()->ObjectTypeInformation;

            for( unsigned osTypeId = 1; osTypeId <= buf.Get()->NumberOfObjectTypes; osTypeId++ )
            {
                CTypeNameMap::iterator it = typenameMap.find( pt->TypeName.Buffer );
                if( it != typenameMap.end() )
                    s_typemap[ it->second ] = osTypeId;
 
                pt = IncBytePtr( pt, AlignDWord( 
                    sizeof(OBJECT_TYPE_INFORMATION) + pt->TypeName.MaximumLength ) );
            }
        } //if
    } //else  

    s_bInitialised = true;
}

//-------------------------------------------------------------------------------------------------

} //namespace NT