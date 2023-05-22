#pragma once

#ifndef CALLBACK_OUTPUT
#define CALLBACK_OUTPUT 0x0
#endif

#ifndef CALLBACK_ERROR
#define CALLBACK_ERROR 0x0d
#endif

typedef struct {
	PCHAR	Original;
	PCHAR	Buffer;
	INT	Length;
	INT	Size;
} DATAP, *PDATAP;

DECLSPEC_IMPORT
VOID
BeaconInjectProcess(
	_In_opt_ HANDLE hProc,
	_In_opt_ INT Pid,
	_In_ PCHAR Payload,
	_In_ INT Length,
	_In_ INT Offset,
	_In_ PVOID Argument,
	_In_ INT ArgLength
);

DECLSPEC_IMPORT
PCHAR
BeaconDataExtract(
	_Inout_ PDATAP Parser,
	_In_ PINT Size
);

DECLSPEC_IMPORT
VOID
BeaconDataParse(
	_Inout_ PDATAP Parser,
	_In_ PCHAR Buffer,
	_In_ INT Size
);

DECLSPEC_IMPORT
SHORT
BeaconDataShort(
    _Inout_ PDATAP Parser
);

DECLSPEC_IMPORT
BOOL
BeaconIsAdmin(
	_In_ VOID
);

DECLSPEC_IMPORT
VOID
BeaconPrintf(
	_In_ INT Type,
	_In_ PCHAR Format,
	...
);

typedef struct {
	D_API( BeaconInjectProcess );
	D_API( BeaconDataExtract );
	D_API( BeaconDataParse );
    D_API( BeaconDataShort );
	D_API( BeaconIsAdmin );
	D_API( BeaconPrintf );
} BAPI_TABLE, *PBAPI_TABLE;
