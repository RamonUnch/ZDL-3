@del *.o
@del zdl.exe

set CFLAGS=-m32 -Os -std=c89 -pedantic -DUNICODE -D_UNICODE -mpreferred-stack-boundary=2 -momit-leaf-frame-pointer -nostdlib -march=i386 -lkernel32 -luser32 -lshell32 -lcomdlg32 -ladvapi32 -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -e_MyMain -D__USE_MINGW_ANSI_STDIO=0 -Wall -flto -fno-ident -fno-exceptions  -fno-dwarf2-cfi-asm -fmerge-all-constants -Wstack-usage=4096 -Wno-unused-parameter -Wsign-compare

windres zdl.rc -o rez.o -Fpe-i386

::gcc -c cfg.c -o cfg.o %CFLAGS%
::gcc -c dlg.c -o dlg.o %CFLAGS%
::gcc -c sub.c -o sub.o %CFLAGS%

gcc zdl.c rez.o cfg.c dlg.c sub.c -o zdl.exe %CFLAGS% -s -mwindows -flto -Wl,--disable-reloc-section -Wl,--disable-runtime-pseudo-reloc -Wl,-nxcompat -Wl,--no-seh -Wl,-s,-dynamicbase

copy /B zdl.exe D:\Jeux\PRTS\ZDL\zdl.exe

@set CFLAGS=
