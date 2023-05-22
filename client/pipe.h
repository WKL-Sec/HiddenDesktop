#pragma once

SECTION( E ) HANDLE PipeInit( PAPI pApi, PCHAR Name );
SECTION( E ) BOOL PipeWait( PAPI pApi, HANDLE Pipe );
SECTION( E ) BOOL PipePrint( PAPI pApi, HANDLE Pipe, PCHAR Format, ... );
