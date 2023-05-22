#pragma once

SECTION( E ) UINT32 HashString( PVOID buffer, ULONG size );
SECTION( E ) PVOID FindModule( ULONG hash, PPEB peb );
SECTION( E ) PVOID FindFunction( PVOID image, ULONG hash );
SECTION( E ) SIZE_T strlenA( PCHAR str );
SECTION( E ) PCHAR strcpyA( PCHAR dst, PCHAR src );
SECTION( E ) PCHAR strcatA( PCHAR dst, PCHAR src );
SECTION( E ) INT strcmpA( PCHAR String1, PCHAR String2 );
