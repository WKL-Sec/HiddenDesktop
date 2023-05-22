#pragma once

#define MAGIC           "WKLWKL"
#define WM_SET_QUALITY  WM_USER + 1

typedef unsigned int        COMPATUINT;
typedef struct __attribute__((packed))
{
    COMPATUINT  msg;
    COMPATUINT  wParam;
    COMPATUINT  lParam;
} MSGPACKET, *PMSGPACKET;

enum Connection
{
    desktop,
    input,
    end
};

enum Quality
{
    QUALITY_LOW     = 10,
    QUALITY_MEDIUM  = 40,
    QUALITY_HIGH    = 100,
};
