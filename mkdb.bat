@del *.o
@del zdld.exe

set CFLAGS=-m32 -Og -std=c89 -pedantic -fanalyzer -gdwarf-2 -D_UNICODE -DUNICODE -municode -march=i686 -lkernel32 -luser32 -lshell32 -lmsvcrt -lcomdlg32 -ladvapi32 -lshlwapi -Wall

windres zdl.rc -o rez.o -Fpe-i386

::gcc -c cfg.c -o cfg.o %CFLAGS%
::gcc -c dlg.c -o dlg.o %CFLAGS%
::gcc -c sub.c -o sub.o %CFLAGS%
::
gcc zdl.c cfg.c dlg.c sub.c rez.o -o zdld.exe %CFLAGS% -mwindows

::@Copy /B zdld.exe D:\Jeux\PRTS\ZDL\zdld.exe
@set CFLAGS=
