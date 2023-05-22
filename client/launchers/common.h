#pragma once

#include <windows.h>
#include <ntstatus.h>

#define D_API( x )    __typeof__( x ) * x
#define U_PTR( x )    ( ( ULONG_PTR ) x )
#define C_PTR( x )    ( ( PVOID ) x )

#define NT_SUCCESS( x )         ( ( NTSTATUS ) ( x ) >= 0 )

#define NtCurrentProcess()      ( ( HANDLE ) (LONG_PTR ) -1 )
#define NtCurrentThread()       ( ( HANDLE )( LONG_PTR ) -2 )

#ifndef memcpy
#define memcpy( destination, source, length ) __builtin_memcpy( destination, source, length );
#endif

#include "../../shared/native.h"
#include "../hashes.h"
#include "../bapi.h"

// https://github.com/realoriginal/foliage/blob/master/source/hash.c
UINT32 HashString( PVOID buffer, ULONG size ) 
{
    UCHAR       Cur = 0;
    ULONG       Djb = 0;
    PUCHAR      Ptr = NULL;

    Djb = 5380;
    Ptr = C_PTR( buffer );
    Djb++;
    
    while ( TRUE )
    {
        Cur = * Ptr;

        if( ! size )
        {
            if( ! * Ptr )
            {
                break;
            };
        } else
        {
            if( ( ULONG )( Ptr - ( PUCHAR )buffer ) >= size )
            {
                break;
            };
            if( ! * Ptr )
            {
                ++Ptr; continue;
            };
        };

        if( Cur >= 'a' )
        {
            Cur -= 0x20;
        };

        Djb = ( ( Djb << 5 ) + Djb ) + Cur; ++Ptr;
    };
    return Djb;
};

// https://github.com/realoriginal/foliage/blob/master/source/peb.c
PVOID FindModule( ULONG hash, PPEB peb )
{
    PLIST_ENTRY             Hdr = NULL;
    PLIST_ENTRY             Ent = NULL;
    PLDR_DATA_TABLE_ENTRY   Ldr = NULL;

    Hdr = & peb->Ldr->InLoadOrderModuleList;
    Ent = Hdr->Flink;

    for( ; Hdr != Ent; Ent = Ent->Flink )
    {
        Ldr = C_PTR( Ent );
        if( HashString( Ldr->BaseDllName.Buffer, Ldr->BaseDllName.Length ) == hash )
        {
            return Ldr->DllBase;
        };
    };
    
    return NULL;
};

// https://github.com/realoriginal/foliage/blob/master/source/pe.c
PVOID FindFunction( PVOID image, ULONG hash ) 
{
    ULONG                       Idx = 0;
    PUINT16                     Aoo = NULL;
    PUINT32                     Aof = NULL;
    PUINT32                     Aon = NULL;
    PIMAGE_DOS_HEADER           Hdr = NULL;
    PIMAGE_NT_HEADERS           Nth = NULL;
    PIMAGE_DATA_DIRECTORY       Dir = NULL;
    PIMAGE_EXPORT_DIRECTORY     Exp = NULL;

    Hdr = C_PTR( image );
    Nth = C_PTR( U_PTR( Hdr ) + Hdr->e_lfanew );
    Dir = & Nth->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];

    if( Dir->VirtualAddress )
    {
        Exp = C_PTR( U_PTR( Hdr ) + Dir->VirtualAddress );
        Aon = C_PTR( U_PTR( Hdr ) + Exp->AddressOfNames );
        Aof = C_PTR( U_PTR( Hdr ) + Exp->AddressOfFunctions );
        Aoo = C_PTR( U_PTR( Hdr ) + Exp->AddressOfNameOrdinals );

        for( Idx = 0 ; Idx < Exp->NumberOfNames ; ++Idx )
        {
            if( HashString( C_PTR( U_PTR( Hdr ) + Aon[ Idx ] ), 0 ) == hash )
            {
                return C_PTR( U_PTR( Hdr ) + Aof[ Aoo[ Idx ] ] );
            };
        };
    };
    return NULL;
};

// https://github.com/vxunderground/VX-API/blob/main/VX-API/StringLength.cpp
SIZE_T strlenA( PCHAR str )
{
    LPCSTR tmp;

    for (tmp = str; *tmp; ++tmp);

    return (tmp - str);
};

// https://github.com/vxunderground/VX-API/blob/main/VX-API/StringCopy.cpp
PCHAR strcpyA( PCHAR dst, PCHAR src )
{
    PCHAR p = dst;

    while ((*p++ = *src++) != 0);

    return dst;
};

// https://github.com/vxunderground/VX-API/blob/main/VX-API/StringConcat.cpp
PCHAR strcatA( PCHAR dst, PCHAR src )
{
    strcpyA(&dst[strlenA(dst)], src);

    return dst;
};

// https://github.com/vxunderground/VX-API/blob/main/VX-API/StringCompare.cpp
INT strcmpA( PCHAR String1, PCHAR String2 )
{
    for ( ; *String1 == *String2; String1++, String2++ )
    {
        if (*String1 == '\0')
        {
            return 0;
        };
    };

    return ( ( *( LPCSTR )String1 < *( LPCSTR )String2 ) ? -1 : +1 );
};
