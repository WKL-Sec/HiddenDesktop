// https://github.com/SolomonSklash/netntlm/blob/master/pipe.c

#include "common.h"

SECTION( E ) HANDLE PipeInit( PAPI pApi, PCHAR Name )
{
    return pApi->kernel32.CreateNamedPipeA( Name,
                                  PIPE_ACCESS_DUPLEX,
                                  PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                  1,
                                  1024 * 1024,
                                  1024 * 1024,
                                  0,
                                  NULL );
};

SECTION( E ) BOOL PipeWait( PAPI pApi, HANDLE Pipe )
{
    if ( !pApi->kernel32.ConnectNamedPipe( Pipe, NULL ) )
    {
        if ( NtCurrentTeb()->LastErrorValue != STATUS_PIPE_CONNECTED  )
        {
            return FALSE;
        };
    };
    return TRUE;
};

SECTION( E ) BOOL PipePrint( PAPI pApi, HANDLE Pipe, PCHAR Format, ... )
{
    INT         Len = 0;
    ULONG       Wrt = 0;
    BOOL        Ret = FALSE;
    PVOID       Str = NULL;
    va_list     Lst = NULL;

    va_start( Lst, Format );
    Len = pApi->msvcrt.vsnprintf( NULL, 0, Format, Lst );
    va_end( Lst );

    if ( ( Str = pApi->kernel32.VirtualAlloc( NULL, Len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE ) ) != NULL )
    {
        va_start( Lst, Format );
        pApi->msvcrt.vsnprintf( Str, Len, Format, Lst );
        va_end( Lst );

        Ret = pApi->kernel32.WriteFile( Pipe, Str, Len + 1, &Wrt, NULL );
        pApi->kernel32.VirtualFree( Str, 0, MEM_RELEASE );
    };
    return Ret;
};
