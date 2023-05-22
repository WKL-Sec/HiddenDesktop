# pragma once

#define SHELLCODE( x )  ( ULONG_PTR )( GetIp( ) - ( ( ULONG_PTR ) & GetIp - ( ULONG_PTR ) x ) )
#define SECTION( x )    __attribute__(( section( ".text$" #x ) ))

#define D_API( x )      __typeof__( x ) * x
#define U_PTR( x )      ( ( ULONG_PTR ) x )
#define C_PTR( x )      ( ( PVOID ) x )

#if defined( _WIN64 )
    #define OFFSET( x )     x
    #define G_END( x )      ( ULONG_PTR )( GetIp() + 11 )
#else
    #define OFFSET( x )     ( ULONG_PTR )( GetIp( ) - ( ( ULONG_PTR ) & GetIp - ( ULONG_PTR ) x ) )
    #define G_END( x )      ( ULONG_PTR )( GetIp() + 10 )
#endif

#define NT_SUCCESS( x )         ( ( NTSTATUS ) ( x ) >= 0 )

#define NtCurrentProcess()      ( ( HANDLE ) (LONG_PTR ) -1 )
#define NtCurrentThread()       ( ( HANDLE )( LONG_PTR ) -2 )

#ifndef memcpy
#define memcpy( destination, source, length ) __builtin_memcpy( destination, source, length );
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM( lp )      ( ( int ) ( short ) LOWORD( lp ) )
#define GET_Y_LPARAM( lp )      ( ( int ) ( short ) HIWORD( lp ) )
#endif
