#include "common.h"

BOOL CW_Register( WNDPROC lpfnWndProc )
{
   WNDCLASSEXA wndClass;
   wndClass.cbSize        = sizeof( WNDCLASSEX );
   wndClass.style         = CS_DBLCLKS;
   wndClass.lpfnWndProc   = lpfnWndProc;
   wndClass.cbClsExtra    = 0;
   wndClass.cbWndExtra    = 0;
   wndClass.hInstance     = NULL;
   wndClass.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
   wndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
   wndClass.hbrBackground = ( HBRUSH )COLOR_WINDOW;
   wndClass.lpszMenuName  = NULL;
   wndClass.lpszClassName = CLASS;
   wndClass.hIconSm       = LoadIcon( NULL, IDI_APPLICATION );

   return RegisterClassEx( &wndClass );
}

HWND CW_Create( DWORD uhid, DWORD width, DWORD height )
{
    PCHAR title = WINDOW_TITLE;
    HWND hWnd = CreateWindowA(
        CLASS, 
        title,
        WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        GetModuleHandle( NULL ),
        NULL );

    if( hWnd == NULL )
    {
        return NULL;
    };

   ShowWindow( hWnd, SW_SHOW ); 
   return hWnd;
};
