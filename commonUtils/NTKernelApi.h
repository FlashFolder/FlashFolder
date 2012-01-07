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
/** \file Some NTDLL.DLL API declarations
 *
 * References:\n 
 *      - "The Undocumented Functions - Microsoft Windows NT/2000"\n 
 *          http://undocumented.ntinternals.net\n
 *      - Book: Windows NT 2000 Native API Reference\n
 *      - Book: Undocumented Windows 2000 Secrets (Schreiber, 2001, ISBN 0-201-72187-2)\n
 *      - NT Device Driver Kit\n
**/

#if !defined(AFX_NTKERNELAPI_H__E051D560_28DE_4B16_B570_ECAA5C6E5301__INCLUDED_)
#define AFX_NTKERNELAPI_H__E051D560_28DE_4B16_B570_ECAA5C6E5301__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

// some of the "documented" NT Kernel API goes here:
#include <winternl.h>

#include <cassert>

//-------------------------------------------------------------------------------------------------

namespace NT
{

//--- data types ---

#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH     ((NTSTATUS)0xC0000004L)


typedef LONG KPRIORITY;

typedef enum _POOL_TYPE 
{
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
	MaxPoolType,
	NonPagedPoolSession = 32,
	PagedPoolSession,
	NonPagedPoolMustSucceedSession,
	DontUseThisTypeSession,
	NonPagedPoolCacheAlignedSession,
	PagedPoolCacheAlignedSession,
	NonPagedPoolCacheAlignedMustSSession
} 
POOL_TYPE;

// -----------------------------------------------------------------

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
} KWAIT_REASON;

//-----------------------------------------------------------------
// Type independent object information

typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,			// Result is OBJECT_BASIC_INFORMATION structure
	ObjectNameInformation,			// Result is OBJECT_NAME_INFORMATION structure
	ObjectTypeInformation,			// Result is OBJECT_TYPE_INFORMATION structure
	ObjectAllTypesInformation,	    // Result is OBJECT_ALL_TYPES_INFORMATION structure
	ObjectDataInformation			// Result is OBJECT_DATA_INFORMATION structure
} 
OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_NAME_INFORMATION 
{
    UNICODE_STRING ObjectName;
} 
OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION 
{
    UNICODE_STRING          TypeName;
    ULONG                   TotalNumberOfHandles;
    ULONG                   TotalNumberOfObjects;
    WCHAR                   Unused1[8];
    ULONG                   HighWaterNumberOfHandles;
    ULONG                   HighWaterNumberOfObjects;
    WCHAR                   Unused2[8];
    ACCESS_MASK             InvalidAttributes;
    GENERIC_MAPPING         GenericMapping;
    ACCESS_MASK             ValidAttributes;
    BOOLEAN                 SecurityRequired;
    BOOLEAN                 MaintainHandleCount;
    USHORT                  MaintainTypeList;
    POOL_TYPE               PoolType;
    ULONG                   DefaultPagedPoolCharge;
    ULONG                   DefaultNonPagedPoolCharge;
} 
OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_ALL_TYPES_INFORMATION 
{
    ULONG                   NumberOfObjectTypes;
    OBJECT_TYPE_INFORMATION ObjectTypeInformation;
        // multiple chunks of variable-length OBJECT_TYPE_INFORMATION
} 
OBJECT_ALL_TYPES_INFORMATION, *POBJECT_ALL_TYPES_INFORMATION;

typedef struct _OBJDIR_INFORMATION 
{
    UNICODE_STRING          ObjectName;
    UNICODE_STRING          ObjectTypeName;
    BYTE                    Data[1];
} 
OBJDIR_INFORMATION, *POBJDIR_INFORMATION;

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)
#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

//-----------------------------------------------------------------

// follows a more comprehensive version of SYSTEM_INFORMATION_CLASS than in <winternl.h>
// reference: www.informit.com, "Interfacing the the Native API in Windows 2000"
typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation,       // 0x002C
    SystemProcessorInformation,     // 0x000C
    SystemPerformanceInformation,    // 0x0138
    SystemTimeInformation,       // 0x0020
    SystemPathInformation,       // not implemented
    SystemProcessInformation,      // 0x00F8+ per process
    SystemCallInformation,       // 0x0018 + (n * 0x0004)
    SystemConfigurationInformation,   // 0x0018
    SystemProcessorCounters,      // 0x0030 per cpu
    SystemGlobalFlag,          // 0x0004
    SystemInfo10,            // not implemented
    SystemModuleInformation,      // 0x0004 + (n * 0x011C)
    SystemLockInformation,       // 0x0004 + (n * 0x0024)
    SystemInfo13,            // not implemented
    SystemPagedPoolInformation,     // checked build only
    SystemNonPagedPoolInformation,   // checked build only
    SystemHandleInformation,      // 0x0004 + (n * 0x0010)
    SystemObjectInformation,      // 0x0038+ + (n * 0x0030+)
    SystemPagefileInformation,     // 0x0018+ per page file
    SystemInstemulInformation,     // 0x0088
    SystemInfo20,            // invalid info class
    SystemCacheInformation,       // 0x0024
    SystemPoolTagInformation,      // 0x0004 + (n * 0x001C)
    SystemProcessorStatistics,     // 0x0000, or 0x0018 per cpu
    SystemDpcInformation,        // 0x0014
    SystemMemoryUsageInformation1,   // checked build only
    SystemLoadImage,          // 0x0018, set mode only
    SystemUnloadImage,         // 0x0004, set mode only
    SystemTimeAdjustmentInformation,  // 0x000C, 0x0008 writeable
    SystemMemoryUsageInformation2,   // checked build only
    SystemInfo30,            // checked build only
    SystemInfo31,            // checked build only
    SystemCrashDumpInformation,     // 0x0004
    SystemExceptionInformation,     // 0x0010
    SystemCrashDumpStateInformation,  // 0x0008
    SystemDebuggerInformation,     // 0x0002
    SystemThreadSwitchInformation,   // 0x0030
    SystemRegistryQuotaInformation,   // 0x000C
    SystemLoadDriver,          // 0x0008, set mode only
    SystemPrioritySeparationInformation,// 0x0004, set mode only
    SystemInfo40,            // not implemented
    SystemInfo41,            // not implemented
    SystemInfo42,            // invalid info class
    SystemInfo43,            // invalid info class
    SystemTimeZoneInformation,     // 0x00AC
    SystemLookasideInformation,     // n * 0x0020
    // info classes specific to Windows 2000
    // WTS = Windows Terminal Server
    SystemSetTimeSlipEvent,       // set mode only
    SystemCreateSession,        // WTS, set mode only
    SystemDeleteSession,        // WTS, set mode only
    SystemInfo49,            // invalid info class
    SystemRangeStartInformation,    // 0x0004
    SystemVerifierInformation,     // 0x0068
    SystemAddVerifier,         // set mode only
    SystemSessionProcessesInformation, // WTS
}
SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

// -----------------------------------------------------------------
// SystemHandleInformation
// reference: Project "FileObjectInfo" by Holger Erne (see www.codeguru.com)

typedef struct _SYSTEM_HANDLE
{
    DWORD       ProcessId;
    BYTE		ObjectType;    // OB_TYPE_*     // OS-Version-dependent, use GetObjectTypeId() !
    BYTE		Flags;         // bits 0..2 HANDLE_FLAG_*
    WORD		Value;         // multiple of 4
	PVOID       pObject;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE, **PPSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG NumberOfHandles;
    SYSTEM_HANDLE Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;


//--- Values for SYSTEM_HANDLE::ObjectType depend on the Windows version

// I have determined the Win2k and XP values in runtime by using NtQueryObject() with 
//      SystemAllTypeInformation and NtQuerySystemInformation() with SystemHandleInformation.
// Since these values seem to change with each new OS version, it's recommended that you
//      use the Information returned by NTQuerySystemInformation() with SystemAllTypeInformation
//      to get the desired type ID if the OS Version is > 5.1
// I have already created a function GetObjectTypeId() that can map a logical object type to its ID
//      that takes the version of the OS into account.

enum LOG_OBJECT_TYPE
{
    OT_Unknown = 0,
    OT_Type,
    OT_Directory,
    OT_SymbolicLink,
    OT_Token,
    OT_Process,
    OT_Thread,
    OT_Job,     
    OT_DebugObject,    
    OT_Event,
    OT_EventPair,
    OT_Mutant,
    OT_Callback,     
    OT_Semaphore,
    OT_Timer,
    OT_Profile,
    OT_KeyedEvent, 
    OT_WindowStation,
    OT_Desktop,
    OT_Section,
    OT_Key,
    OT_Port,
    OT_WaitablePort,    
    OT_Adapter,
    OT_Controller,
    OT_Device,
    OT_Driver,
    OT_IO_Completion,
    OT_File,
    OT_WmiGuid,
    _OT_MaxType
};

enum OBJECT_TYPE_XP   // Windows 2000 and above
{
    OTXP_Type            = 0x01,
    OTXP_Directory       = 0x02,
    OTXP_SymbolicLink    = 0x03,
    OTXP_Token           = 0x04,
    OTXP_Process         = 0x05,
    OTXP_Thread          = 0x06,
    OTXP_Job             = 0x07,     
    OTXP_DebugObject     = 0x08,    // new compared to 2k    
    OTXP_Event           = 0x09,
    OTXP_EventPair       = 0x0A,
    OTXP_Mutant          = 0x0B,
    OTXP_Callback        = 0x0C,     
    OTXP_Semaphore       = 0x0D,
    OTXP_Timer           = 0x0E,
    OTXP_Profile         = 0x0F,
    OTXP_KeyedEvent      = 0x10,    // new compared to 2k
    OTXP_WindowStation   = 0x11,
    OTXP_Desktop         = 0x12,
    OTXP_Section         = 0x13,
    OTXP_Key             = 0x14,
    OTXP_Port            = 0x15,
    OTXP_WaitablePort    = 0x16,    
    OTXP_Adapter         = 0x17,
    OTXP_Controller      = 0x18,
    OTXP_Device          = 0x19,
    OTXP_Driver          = 0x1A,
    OTXP_IO_Completion   = 0x1B,
    OTXP_File            = 0x1C,
    OTXP_WmiGuid         = 0x1D     // new compared to 2k
};

enum OBJECT_TYPE_2K   // Windows 2000 and above
{
    OT2K_Type            = 0x01,
    OT2K_Directory       = 0x02,
    OT2K_SymbolicLink    = 0x03,
    OT2K_Token           = 0x04,
    OT2K_Process         = 0x05,
    OT2K_Thread          = 0x06,
    OT2K_Job             = 0x07,     // new compared to NT4
    OT2K_Event           = 0x08,
    OT2K_EventPair       = 0x09,
    OT2K_Mutant          = 0x0A,
    OT2K_Callback        = 0x0B,     // new compared to NT4
    OT2K_Semaphore       = 0x0C,
    OT2K_Timer           = 0x0D,
    OT2K_Profile         = 0x0E,
    OT2K_WindowStation   = 0x0F,
    OT2K_Desktop         = 0x10,
    OT2K_Section         = 0x11,
    OT2K_Key             = 0x12,
    OT2K_Port            = 0x13,
    OT2K_WaitablePort    = 0x14,     // new compared to NT4
    OT2K_Adapter         = 0x15,
    OT2K_Controller      = 0x16,
    OT2K_Device          = 0x17,
    OT2K_Driver          = 0x18,
    OT2K_IO_Completion   = 0x19,
    OT2K_File            = 0x1A
};

enum OBJECT_TYPE_NT4    // Windows NT 4 and below
{
    OT4_Type            = 0x01,
    OT4_Directory       = 0x02,
    OT4_SymbolicLink    = 0x03,
    OT4_Token           = 0x04,
    OT4_Process         = 0x05,
    OT4_Thread          = 0x06,
    OT4_Event           = 0x07,
    OT4_EventPair       = 0x08,
    OT4_Mutant          = 0x09,
    OT4_Semaphore       = 0x10,
    OT4_Timer           = 0x11,
    OT4_Profile         = 0x12,
    OT4_WindowStation   = 0x13,
    OT4_Desktop         = 0x14,
    OT4_Section         = 0x15,
    OT4_Key             = 0x16,
    OT4_Port            = 0x17,
    OT4_Adapter         = 0x18,
    OT4_Controller      = 0x19,
    OT4_Device          = 0x20,
    OT4_Driver          = 0x21,
    OT4_IO_Completion   = 0x22,
    OT4_File            = 0x23  
};

//--------------------------------------------------------------------
// SystemProcessInformation

typedef struct _CLIENT_ID {
    ULONG UniqueProcess;
    ULONG UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG        PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREAD
    {
/*000*/ FILETIME   ftKernelTime;   // 100 nsec units
/*008*/ FILETIME   ftUserTime;    // 100 nsec units
/*010*/ FILETIME   ftCreateTime;   // relative to 01-01-1601
/*018*/ DWORD    dWaitTime;
/*01C*/ PVOID    pStartAddress;
/*020*/ CLIENT_ID  Cid;        // process/thread ids
/*028*/ DWORD    dPriority;
/*02C*/ DWORD    dBasePriority;
/*030*/ DWORD    dContextSwitches;
/*034*/ DWORD    dThreadState;   // 2=running, 5=waiting
/*038*/ KWAIT_REASON WaitReason;
/*03C*/ DWORD    dReserved01;
/*040*/ }
    SYSTEM_THREAD, *PSYSTEM_THREAD;
#define SYSTEM_THREAD_ sizeof (SYSTEM_THREAD)

typedef struct _SYSTEM_PROCESS     // common members
    {
/*000*/ DWORD     dNext;      // relative offset
/*004*/ DWORD     dThreadCount;
/*008*/ DWORD     dReserved01;
/*00C*/ DWORD     dReserved02;
/*010*/ DWORD     dReserved03;
/*014*/ DWORD     dReserved04;
/*018*/ DWORD     dReserved05;
/*01C*/ DWORD     dReserved06;
/*020*/ FILETIME    ftCreateTime;  // relative to 01-01-1601
/*028*/ FILETIME    ftUserTime;   // 100 nsec units
/*030*/ FILETIME    ftKernelTime;  // 100 nsec units
/*038*/ UNICODE_STRING usName;
/*040*/ KPRIORITY   BasePriority;
/*044*/ DWORD     dUniqueProcessId;
/*048*/ DWORD     dInheritedFromUniqueProcessId;
/*04C*/ DWORD     dHandleCount;
/*050*/ DWORD     dReserved07;
/*054*/ DWORD     dReserved08;
/*058*/ VM_COUNTERS  VmCounters;   // see ntddk.h
/*084*/ DWORD     dCommitCharge;  // bytes
/*088*/ }
    SYSTEM_PROCESS, *PSYSTEM_PROCESS;
#define SYSTEM_PROCESS_ sizeof (SYSTEM_PROCESS)
    
typedef struct _SYSTEM_PROCESS_NT4   // Windows NT 4.0
    {
/*000*/ SYSTEM_PROCESS Process;     // common members
/*088*/ SYSTEM_THREAD aThreads[1];   // thread array
/*088*/ }
    SYSTEM_PROCESS_NT4, *PSYSTEM_PROCESS_NT4;
#define SYSTEM_PROCESS_NT4_ sizeof (SYSTEM_PROCESS_NT4)

typedef struct _SYSTEM_PROCESS_NT5   // Windows 2000
    {
/*000*/ SYSTEM_PROCESS Process;     // common members
/*088*/ IO_COUNTERS  IoCounters;   // see ntddk.h
/*0B8*/ SYSTEM_THREAD aThreads[1];   // thread array
/*0B8*/ }
    SYSTEM_PROCESS_NT5, *PSYSTEM_PROCESS_NT5;
#define SYSTEM_PROCESS_NT5_ sizeof (SYSTEM_PROCESS_NT5)

typedef union _SYSTEM_PROCESS_INFORMATION
    {
/*000*/ SYSTEM_PROCESS   Process;
/*000*/ SYSTEM_PROCESS_NT4 Process_NT4;
/*000*/ SYSTEM_PROCESS_NT5 Process_NT5;
/*0B8*/ }
    SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

//----------------------------------------------------------------------
// File Objects

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,//2
    FileBothDirectoryInformation,//3
    FileBasicInformation,//4
    FileStandardInformation,//5
    FileInternalInformation,//6
    FileEaInformation,//7
    FileAccessInformation,//8
    FileNameInformation,//9
    FileRenameInformation,//10 (0Ah)
    FileLinkInformation,//11 (0Bh)
    FileNamesInformation,//12 (0Ch)
    FileDispositionInformation,//13 (0Dh)
    FilePositionInformation,//14 (0Eh)
    FileFullEaInformation,//15 (0Fh)
    FileModeInformation,//16 (10h)
    FileAlignmentInformation,//17 (11h)
    FileAllInformation,//18 (12h)
    FileAllocationInformation,//19 (13h)
    FileEndOfFileInformation,//20 (14h)
    FileAlternateNameInformation,//21 (15h)
    FileStreamInformation,//22 (16h)
    FilePipeInformation,//23 (17h)
    FilePipeLocalInformation,//24 (18h)
    FilePipeRemoteInformation,//25 (19h)
    FileMailslotQueryInformation,//26 (1Ah)
    FileMailslotSetInformation,//27 (1Bh)
    FileCompressionInformation,//28 (1Ch)
    FileCopyOnWriteInformation,//29 (1Dh)
    FileCompletionInformation,//30 (1Eh)
    FileMoveClusterInformation,//31 (1Fh)
    FileOleClassIdInformation,//32 (20h)
    FileOleStateBitsInformation,//33 (21h)
    FileApplicationExplorableInformation,//34 (22h)
    FileApplicationExplorableChildrenInformation,//35 (23h)
    FileObjectIdInformation,//36 (24h)
    FileOleAllInformation,//37 (25h)
    FileOleDirectoryInformation,//38 (26h)
    FileTransactionCommitInformation,//39 (27h)
    FileContentIndexInformation,//40 (28h)
    FileInheritContentIndexInformation,//41 (29h)
    FileOleInformation,//42 (2Ah)
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _FILE_NAME_INFORMATION {                     
  ULONG  FileNameLength;                                   
  WCHAR  FileName[1];                                      
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;   

typedef struct _FILE_BASIC_INFORMATION {                    
    LARGE_INTEGER CreationTime;                             
    LARGE_INTEGER LastAccessTime;                           
    LARGE_INTEGER LastWriteTime;                            
    LARGE_INTEGER ChangeTime;                               
    ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;  

// some file-related flags 
/*
const DWORD FILE_ANY_ACCESS                     = 0x0000; // any type
const DWORD FILE_READ_ACCESS                    = 0x0001; // file & pipe
const DWORD FILE_READ_DATA                      = 0x0001; // file & pipe
const DWORD FILE_LIST_DIRECTORY                 = 0x0001; // directory
const DWORD FILE_WRITE_ACCESS                   = 0x0002; // file & pipe
const DWORD FILE_WRITE_DATA                     = 0x0002; // file & pipe
const DWORD FILE_ADD_FILE                       = 0x0002; // directory
const DWORD FILE_APPEND_DATA                    = 0x0004; // file
const DWORD FILE_ADD_SUBDIRECTORY               = 0x0004; // directory
const DWORD FILE_CREATE_PIPE_INSTANCE           = 0x0004; // named pipe
const DWORD FILE_READ_EA                        = 0x0008; // file & directory

const DWORD FILE_WRITE_EA                       = 0x0010; // file & directory
const DWORD FILE_EXECUTE                        = 0x0020; // file
const DWORD FILE_TRAVERSE                       = 0x0020; // directory
const DWORD FILE_DELETE_CHILD                   = 0x0040; // directory
const DWORD FILE_READ_ATTRIBUTES                = 0x0080; // all types

const DWORD FILE_WRITE_ATTRIBUTES               = 0x0100; // all types
const DWORD FILE_ALL_ACCESS                     (= 0x01FF | STANDARD_RIGHTS_ALL)
*/
const DWORD FILE_DIRECTORY_FILE                     = 0x00000001;
const DWORD FILE_WRITE_THROUGH                      = 0x00000002;
const DWORD FILE_SEQUENTIAL_ONLY                    = 0x00000004;
const DWORD FILE_NO_INTERMEDIATE_BUFFERING          = 0x00000008;

const DWORD FILE_SYNCHRONOUS_IO_ALERT               = 0x00000010;
const DWORD FILE_SYNCHRONOUS_IO_NONALERT            = 0x00000020;
const DWORD FILE_NON_DIRECTORY_FILE                 = 0x00000040;
const DWORD FILE_CREATE_TREE_CONNECTION             = 0x00000080;

const DWORD FILE_COMPLETE_IF_OPLOCKED               = 0x00000100;
const DWORD FILE_NO_EA_KNOWLEDGE                    = 0x00000200;
const DWORD FILE_DISABLE_TUNNELING                  = 0x00000400;
const DWORD FILE_RANDOM_ACCESS                      = 0x00000800;

const DWORD FILE_DELETE_ON_CLOSE                    = 0x00001000;
const DWORD FILE_OPEN_BY_FILE_ID                    = 0x00002000;
const DWORD FILE_OPEN_FOR_BACKUP_INTENT             = 0x00004000;
const DWORD FILE_NO_COMPRESSION                     = 0x00008000;

const DWORD FILE_VALID_OPTION_FLAGS                 = 0x000FFFFF;

const DWORD FILE_VALID_SET_FLAGS                    = 0x00001036;


//=================================================================================================
// function types for manual dynamic DLL linking at runtime follow
//=================================================================================================

typedef NTSTATUS (NTAPI* PNtQuerySystemInformation)(
  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
  OUT PVOID               SystemInformation,
  IN ULONG                SystemInformationLength,
  OUT PULONG              ReturnLength OPTIONAL );

typedef NTSTATUS (NTAPI* PNtQueryObject)( 
  IN HANDLE               ObjectHandle OPTIONAL,
  IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
  OUT PVOID               ObjectInformation,
  IN ULONG                Length,
  OUT PULONG              ResultLength );

typedef NTSTATUS (NTAPI* PNtOpenDirectoryObject)(
  OUT PHANDLE             DirectoryObjectHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes );

typedef NTSTATUS (NTAPI* PNtQueryDirectoryObject)(
  IN HANDLE               DirectoryObjectHandle,
  OUT POBJDIR_INFORMATION DirObjInformation,
  IN ULONG                BufferLength,
  IN BOOLEAN              GetNextIndex,
  IN BOOLEAN              IgnoreInputIndex,
  IN OUT PULONG           ObjectIndex,
  OUT PULONG              DataWritten OPTIONAL );

typedef NTSTATUS (NTAPI* PNtClose)( HANDLE ObjectHandle );

typedef NTSTATUS (NTAPI* PNtCreateFile)( 
  OUT PHANDLE             FileHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN PLARGE_INTEGER       AllocationSize OPTIONAL,
  IN ULONG                FileAttributes,
  IN ULONG                ShareAccess,
  IN ULONG                CreateDisposition,
  IN ULONG                CreateOptions,
  IN PVOID                EaBuffer OPTIONAL,
  IN ULONG                EaLength );

typedef NTSTATUS (NTAPI* PNtOpenFile)(
  OUT PHANDLE             FileHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN ULONG                ShareAccess,
  IN ULONG                OpenOptions );

typedef NTSTATUS (NTAPI* PNtQueryInformationFile)(
  IN HANDLE               FileHandle,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  OUT PVOID               FileInformation,
  IN ULONG                Length,
  IN FILE_INFORMATION_CLASS FileInformationClass );

typedef NTSTATUS (NTAPI* PNtSetInformationFile)(
  IN HANDLE               FileHandle,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN PVOID                FileInformation,
  IN ULONG                Length,
  IN FILE_INFORMATION_CLASS FileInformationClass );

typedef NTSTATUS (NTAPI* PNtQueryAttributesFile)(
  IN POBJECT_ATTRIBUTES       ObjectAttributes,
  OUT PFILE_BASIC_INFORMATION FileAttributes );

//-------------------------------------------------------------------------------------------------
// some redirector functions for automatic dynamic linking to the DLL functions follow
// purpose is to have no redundant calls to ::GetModuleHandle() and ::GetProcAddress()
//-------------------------------------------------------------------------------------------------

template <typename T> T GetDllProc( HMODULE hMod, char* pFuncName )
{ 
    assert( hMod );
    T pFunc = reinterpret_cast<T>( ::GetProcAddress( hMod, pFuncName ) ); 
    assert( pFunc );
    return pFunc;
}

//-------------------------------------------------------------------------------------------------

extern HMODULE g_hNtDll;

inline NTSTATUS NTAPI D_NtQuerySystemInformation(
  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
  OUT PVOID               SystemInformation,
  IN ULONG                SystemInformationLength,
  OUT PULONG              ReturnLength OPTIONAL )
{
    extern PNtQuerySystemInformation g_pNtQuerySystemInformation;
    if( ! g_pNtQuerySystemInformation )
        g_pNtQuerySystemInformation = GetDllProc<PNtQuerySystemInformation>( 
                g_hNtDll, "NtQuerySystemInformation" );
    
    return g_pNtQuerySystemInformation( 
        SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength );    
}

inline NTSTATUS NTAPI D_NtQueryObject( 
  IN HANDLE               ObjectHandle OPTIONAL,
  IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
  OUT PVOID               ObjectInformation,
  IN ULONG                Length,
  OUT PULONG              ResultLength )
{
    extern PNtQueryObject g_pNtQueryObject;
    if( ! g_pNtQueryObject )
        g_pNtQueryObject = GetDllProc<PNtQueryObject>( g_hNtDll, "NtQueryObject" );
    
    return g_pNtQueryObject( 
        ObjectHandle, ObjectInformationClass, ObjectInformation, Length, ResultLength );        
}

inline NTSTATUS NTAPI D_NtOpenDirectoryObject(
  OUT PHANDLE             DirectoryObjectHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes )
{
    extern PNtOpenDirectoryObject g_pNtOpenDirectoryObject;
    if( ! g_pNtOpenDirectoryObject )
        g_pNtOpenDirectoryObject = GetDllProc<PNtOpenDirectoryObject>( 
                g_hNtDll, "NtOpenDirectoryObject" );
    
    return g_pNtOpenDirectoryObject( 
        DirectoryObjectHandle, DesiredAccess, ObjectAttributes );        
}

inline NTSTATUS NTAPI D_NtQueryDirectoryObject(
  IN HANDLE               DirectoryObjectHandle,
  OUT POBJDIR_INFORMATION DirObjInformation,
  IN ULONG                BufferLength,
  IN BOOLEAN              GetNextIndex,
  IN BOOLEAN              IgnoreInputIndex,
  IN OUT PULONG           ObjectIndex,
  OUT PULONG              DataWritten OPTIONAL )
{
    extern PNtQueryDirectoryObject g_pNtQueryDirectoryObject;
    if( ! g_pNtQueryDirectoryObject )
        g_pNtQueryDirectoryObject = GetDllProc<PNtQueryDirectoryObject>( 
                g_hNtDll, "NtQueryDirectoryObject" );
    
    return g_pNtQueryDirectoryObject( 
        DirectoryObjectHandle, DirObjInformation, BufferLength, GetNextIndex, IgnoreInputIndex,
        ObjectIndex, DataWritten );        
}

inline NTSTATUS NTAPI D_NtClose( HANDLE ObjectHandle )
{
    extern PNtClose g_pNtClose;
    if( ! g_pNtClose )
        g_pNtClose = GetDllProc<PNtClose>( g_hNtDll, "NtClose" );
    
    return g_pNtClose( ObjectHandle );        
}

inline NTSTATUS NTAPI D_NtCreateFile( 
  OUT PHANDLE             FileHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN PLARGE_INTEGER       AllocationSize OPTIONAL,
  IN ULONG                FileAttributes,
  IN ULONG                ShareAccess,
  IN ULONG                CreateDisposition,
  IN ULONG                CreateOptions,
  IN PVOID                EaBuffer OPTIONAL,
  IN ULONG                EaLength )
{
    extern PNtCreateFile g_pNtCreateFile;
    if( ! g_pNtCreateFile )
        g_pNtCreateFile = GetDllProc<PNtCreateFile>( g_hNtDll, "NtCreateFile" );
    
    return g_pNtCreateFile( 
        FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
        FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength );        
}

inline NTSTATUS NTAPI D_NtOpenFile(
  OUT PHANDLE             FileHandle,
  IN ACCESS_MASK          DesiredAccess,
  IN POBJECT_ATTRIBUTES   ObjectAttributes,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN ULONG                ShareAccess,
  IN ULONG                OpenOptions )
{
    extern PNtOpenFile g_pNtOpenFile;
    if( ! g_pNtOpenFile )
        g_pNtOpenFile = GetDllProc<PNtOpenFile>( g_hNtDll, "NtOpenFile" );
    
    return g_pNtOpenFile( 
        FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions );        
}

inline NTSTATUS NTAPI D_NtQueryInformationFile(
  IN HANDLE               FileHandle,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  OUT PVOID               FileInformation,
  IN ULONG                Length,
  IN FILE_INFORMATION_CLASS FileInformationClass )
{
    extern PNtQueryInformationFile g_pNtQueryInformationFile;
    if( ! g_pNtQueryInformationFile )
        g_pNtQueryInformationFile = GetDllProc<PNtQueryInformationFile>( 
                g_hNtDll, "NtQueryInformationFile" );
    
    return g_pNtQueryInformationFile( 
        FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );    
}

inline NTSTATUS NTAPI D_NtSetInformationFile(
  IN HANDLE               FileHandle,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN PVOID                FileInformation,
  IN ULONG                Length,
  IN FILE_INFORMATION_CLASS FileInformationClass )
{
    extern PNtSetInformationFile g_pNtSetInformationFile;
    if( ! g_pNtSetInformationFile )
        g_pNtSetInformationFile = GetDllProc<PNtSetInformationFile>( 
                g_hNtDll, "NtSetInformationFile" );
    
    return g_pNtSetInformationFile( 
        FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );    
}

inline NTSTATUS NTAPI D_NtQueryAttributesFile(
  IN POBJECT_ATTRIBUTES       ObjectAttributes,
  OUT PFILE_BASIC_INFORMATION FileAttributes )
{
    extern PNtQueryAttributesFile g_pNtQueryAttributesFile;
    if( ! g_pNtQueryAttributesFile )
        g_pNtQueryAttributesFile = GetDllProc<PNtQueryAttributesFile>( 
                g_hNtDll, "NtQueryAttributesFile" );
    
    return g_pNtQueryAttributesFile( ObjectAttributes, FileAttributes );       
}

//-------------------------------------------------------------------------------------------------

// Helper class maps logical object types to their OS-version-dependent object type IDs
// these IDs are returned in the SYSTEM_HANDLE::ObjectType when calling NtQuerySystemInformation() 
//    with SystemHandleInformation 

class CNtObjTypeMap
{
public:
    static inline BYTE GetTypeId( LOG_OBJECT_TYPE objectType )
    {
        if( ! s_bInitialised ) Init();
        return s_typemap[objectType]; 
    }

private:
    static void Init();    

    static bool s_bInitialised;
    static BYTE s_typemap[_OT_MaxType];
};

//--------------------------------------------------------------------------------------------

// Helper class makes working with return buffers for the various NtQuery... functions easier. 
template<class T> class CTypedBuf
{
public:
    T type;

    CTypedBuf( unsigned sizeBytes = 0 ) : m_pBuf(NULL)
    { 
        if( sizeBytes ) m_pBuf = new char[sizeBytes];
        m_size = sizeBytes;           
    }
    ~CTypedBuf() { delete[] m_pBuf; }
    void Free()                     
    { 
        delete[] m_pBuf; 
        m_pBuf = NULL; m_size = 0;
    }
    T* Get() { return reinterpret_cast<T*>( m_pBuf ); }
    const T* Get() const { return reinterpret_cast<T*>( m_pBuf ); }
    unsigned GetSize() const { return m_size; }
    void Resize( unsigned sizeBytes )    
    { 
        // note: Resize() will discard the old contents of the memory in *m_pBuf !
        Free();
        if( sizeBytes )
        {
            m_pBuf = new char[sizeBytes];
            m_size = sizeBytes;
        }
    }
private:
    unsigned m_size;
    char* m_pBuf;
};

//--------------------------------------------------------------------------------------------

}; //namespace NT

#endif // !defined(AFX_NTKERNELAPI_H__E051D560_28DE_4B16_B570_ECAA5C6E5301__INCLUDED_)
