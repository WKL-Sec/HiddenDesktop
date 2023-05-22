#pragma once

typedef struct
{
    PAPI        pApi;
    PCHAR       pPipeName;
    PCHAR       pDesktopName;
    SHORT       port;
    HANDLE      log;
    ULONG_PTR   gdiplusToken;
    COLORREF    color;
    ULONG       quality;
    HDESK       hDesktop;
    BITMAPINFO  bmpInfo;
    PBYTE       pPixels;
    PBYTE       pOldPixels;
    PBYTE       pTempPixels;

} ARGS, *PARGS;
