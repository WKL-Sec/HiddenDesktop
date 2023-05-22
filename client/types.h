#pragma once

typedef HDC *PHDC;
typedef HBITMAP *PHBITMAP;
typedef CLSID *PCLSID;
typedef GpBitmap GPBITMAP;
typedef GPBITMAP *PGPBITMAP;
typedef IStream *PISTREAM;
typedef EncoderParameters ENCODERPRM;

typedef BOOL ( CALLBACK* PICWNDENUMPROC )(HWND, LPARAM, PAPI );

typedef struct 
{
    HDC hDc;
    HDC hDcScreen;
} WINDOWS, *PWINDOWS;
