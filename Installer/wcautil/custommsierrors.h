#pragma once
//---------------------------------------------------------------------------
// CustomMsiErrors.h
// Indexes for office custom errors in the MSI
// 
// Created 3/12/00  AndrewH - RADDev 
//
// Note: Custom Errors must be in the range 25000-30000, all other error
//   codes are reserved for the Windows Installer as standard error ranges
//   NEVER reuse an error number or you're likely to break the builds.
//
//---------------------------------------------------------------------------

// Instructions:
//    1. add the index to this file
//    2. define the error table entry in msi\src\sdl\misc\CustomErrors.sdl
//    3. #include CustomMsiErrors to refer to the index
//    4. Import Misc\CustomErrors { MYDEFINE=1 };  with your errorgroup under MYDEFINE


//---------------------------------------------------------------------------
// Office Core 25000-25089

#define OFFICE_ERROR_BASE               25000

#define msierrFirstRunNotAdmin          25002
#define msierrMsiidSourceConsistency    25003

//---------------------------------------------------------------------------
// LocalCache 25090-25099

#define LOCALCACHE_ERROR_BASE           25090

#define msierrFailedInitializeInjector  25090
#define msierrFailedChangeSource        25091

//---------------------------------------------------------------------------
// OWS    25101-25200
#define OWS_ERROR_BASE                  25101

#define msierrOws1                      25101

// STSTPKPL 25111-25120 -- within STS range
#define msierrStstpkplNeeds_STS2	25111
#define msierrNoLaunchFromChainer	25112

//---------------------------------------------------------------------------
// O10 91581: put up nice error dialog on particular error calling IE
// OSP    25201-25220
#define OSP_ERROR_BASE                  25201

#define msierrIE5SetupNotAvailable      25201
#define msierrIE5SetupCannotInstall     25202


//---------------------------------------------------------------------------
// SKU variation
// SKU features' range    25301-25400
#define SKU_ERROR_BASE                  25301

#define msierrAdminNotSupported         25301
#define msierrQuietNotSupported         25302
#define msierrCIWNotSupported           25303
#define msierrPidkeyRequired            25304
#define msierrOfficeLiteOverFull        25305
#define msierrCMWNotSupported           25306

//---------------------------------------------------------------------------
// ORK    25401-25450
#define ORK_ERROR_BASE                  25401

#define msierrOrkPolEditSql             25401
#define msierrOrkPolEditSql2            25402
#define msierrOrkPolEditData            25403
#define msierrOrkPolEditReg             25404

//---------------------------------------------------------------------------
// MUI 25451 - 25500
#define MUI_ERROR_BASE		25451

#define msierrMUINotSupported	25452

//---------------------------------------------------------------------------
// GLOBAL    25501-25600
#define GLOBAL_ERROR_BASE                 25501

#define msierrUrlTransformReq             25501
#define msierrFeatureDepSql               25502
#define msierrFeatureDepReadData          25503
#define msierrFeatureDepFailed            25504
#define msierrTSPidkeyRequired            25505
#define msierrOSPCheckInsufficientVersion 25506
#define msierrOSPCheckNoAccess            25507
#define msierrOSPCheckAbsent              25508
#define msierrOSPCheckAbsentOrNoAccess    25509
#define msierrOSPCheckMissing             25510

#define msierrSecureObjectsFailedCreateSD    25520
#define msierrSecureObjectsFailedSet         25521
#define msierrSecureObjectsUnknownType       25522

#define msierrXmlFileFailedRead         25530
#define msierrXmlFileFailedOpen         25531
#define msierrXmlFileFailedSelect       25532
#define msierrXmlFileFailedSave         25533

#define msierrXmlConfigFailedRead         25540
#define msierrXmlConfigFailedOpen         25541
#define msierrXmlConfigFailedSelect       25542
#define msierrXmlConfigFailedSave         25543

//---------------------------------------------------------------------------
// Server CustomAction Errors
// SERVER range: 26001-26100
#define SERVER_ERROR_BASE                      26000

#define msierrIISCannotConnect                 26001
#define msierrIISFailedReadWebSite             26002
#define msierrIISFailedReadWebDirs             26003
#define msierrIISFailedReadVDirs               26004
#define msierrIISFailedReadFilters             26005
#define msierrIISFailedReadAppPool             26006
#define msierrIISFailedReadMimeMap             26007
#define msierrIISFailedReadProp                26008
#define msierrIISFailedReadWebSvcExt           26009
#define msierrIISFailedReadWebError            26010
#define msierrIISFailedReadHttpHeader          26011

#define msierrIISFailedSchedTransaction        26031
#define msierrIISFailedSchedInstallWebs        26032
#define msierrIISFailedSchedInstallWebDirs     26033
#define msierrIISFailedSchedInstallVDirs       26034
#define msierrIISFailedSchedInstallFilters     26035
#define msierrIISFailedSchedInstallAppPool     26036
#define msierrIISFailedSchedInstallProp        26037
#define msierrIISFailedSchedInstallWebSvcExt   26038

#define msierrIISFailedSchedUninstallWebs      26051
#define msierrIISFailedSchedUninstallWebDirs   26052
#define msierrIISFailedSchedUninstallVDirs     26053
#define msierrIISFailedSchedUninstallFilters   26054
#define msierrIISFailedSchedUninstallAppPool   26055
#define msierrIISFailedSchedUninstallProp      26056
#define msierrIISFailedSchedUninstallWebSvcExt 26057

#define msierrIISFailedStartTransaction        26101
#define msierrIISFailedOpenKey                 26102
#define msierrIISFailedCreateKey               26103
#define msierrIISFailedWriteData               26104
#define msierrIISFailedCreateApp               26105
#define msierrIISFailedDeleteKey               26106

#define msierrSQLFailedCreateDatabase          26201
#define msierrSQLFailedDropDatabase            26202
#define msierrSQLFailedConnectDatabase         26203
#define msierrSQLFailedExecString              26204
#define msierrSQLDatabaseAlreadyExists         26205

#define msierrPERFMONFailedRegisterDLL         26251
#define msierrPERFMONFailedUnregisterDLL       26252
#define msierrInstallPerfCounterData           26253
#define msierrUninstallPerfCounterData         26254

#define msierrSMBFailedCreate                  26301
#define msierrSMBFailedDrop                    26302

#define msierrCERTFailedOpen                   26351
#define msierrCERTFailedAdd                    26352

#define msierrUSRFailedUserCreate              26401
#define msierrUSRFailedUserCreatePswd          26402
#define msierrUSRFailedUserGroupAdd            26403
#define msierrUSRFailedUserCreateExists        26404
#define msierrUSRFailedGrantLogonAsService     26405



//--------------------------------------------------------------------------
// Managed code CustomAction Errors
// MANAGED range: 27000-27100
#define MANAGED_ERROR_BASE                     27000

#define msierrDotNetRuntimeRequired            27000


//---------------------------------------------------------------------------
// Public CustomAction Errors
// PUBLIC range: 28001-28100
#define PUBLIC_ERROR_BASE                            28000

#define msierrComPlusCannotConnect                   28001
#define msierrComPlusPartitionReadFailed             28002
#define msierrComPlusPartitionRoleReadFailed         28003
#define msierrComPlusUserInPartitionRoleReadFailed   28004
#define msierrComPlusPartitionUserReadFailed         28005
#define msierrComPlusApplicationReadFailed           28006
#define msierrComPlusApplicationRoleReadFailed       28007
#define msierrComPlusUserInApplicationRoleReadFailed 28008
#define msierrComPlusAssembliesReadFailed            28009
#define msierrComPlusSubscriptionReadFailed          28010
#define msierrComPlusPartitionDependency             28011
#define msierrComPlusPartitionNotFound               28012
#define msierrComPlusPartitionIdConflict             28013
#define msierrComPlusPartitionNameConflict           28014
#define msierrComPlusApplicationDependency           28015
#define msierrComPlusApplicationNotFound             28016
#define msierrComPlusApplicationIdConflict           28017
#define msierrComPlusApplicationNameConflict         28018
#define msierrComPlusApplicationRoleDependency       28019
#define msierrComPlusApplicationRoleNotFound         28020
#define msierrComPlusApplicationRoleConflict         28021
#define msierrComPlusAssemblyDependency              28022
#define msierrComPlusSubscriptionIdConflict          28023
#define msierrComPlusSubscriptionNameConflict        28024
#define msierrComPlusFailedLookupNames               28025
