@del *.o
@del zdld.exe

set CFLAGS=-m32 -Og  -fanalyzer -ggdb -g -std=gnu89 -D_UNICODE -DUNICODE -nostdlib -march=i686 -lkernel32 -luser32 -lshell32 -lmsvcrt -lcomdlg32 -ladvapi32 -lshlwapi -e_MyMain -D__USE_MINGW_ANSI_STDIO=0 -Wall

windres zdl.rc -o rez.o -Fpe-i386

::gcc -c cfg.c -o cfg.o %CFLAGS%
::gcc -c dlg.c -o dlg.o %CFLAGS%
::gcc -c sub.c -o sub.o %CFLAGS%
::
gcc zdl.c cfg.c dlg.c sub.c rez.o -o zdl.exe %CFLAGS% -mwindows -Wl,-nxcompat -Wl,--no-seh -Wl,-dynamicbase

::@Copy /B zdl.exe D:\Jeux\PRTS\ZDL\zdl3.exe
@set CFLAGS=
