#pragma once

#include <winsock2.h>
#include <windows.h>
#include <ntstatus.h>
#include <stdio.h>
#include <gdiplus.h>

#include "macros.h"
#include "../shared/native.h"
#include "../shared/config.h"
#include "hashes.h"
#include "bapi.h"
#include "util.h"
#include "bfmain.h"
#include "api.h"
#include "args.h"
#include "types.h"
#include "scmain.h"
#include "pipe.h"

extern ULONG_PTR GetIp( VOID );
extern ULONG_PTR Leave( VOID );
