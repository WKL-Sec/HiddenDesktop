[SEGMENT .text]

%ifidn __OUTPUT_FORMAT__, win32
    GLOBAL  _BofMain
    _BofMain:
    incbin "bin/HiddenDesktop.x86.bin"
%else
    GLOBAL  BofMain
    BofMain:
    incbin "bin/HiddenDesktop.x64.bin"
%endif
