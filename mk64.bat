@del *.o
@del zdl64.exe

set CFLAGS=-m64 -Os -std=c89 -DUNICODE -D_UNICODE -momit-leaf-frame-pointer -nostdlib -lkernel32 -luser32 -lshell32 -lcomdlg32 -ladvapi32 -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -eMyMain -D__USE_MINGW_ANSI_STDIO=0 -Wall -flto -fno-ident -fno-exceptions  -fno-dwarf2-cfi-asm -fmerge-all-constants -Wstack-usage=4096 -Wno-unused-parameter

windres zdl.rc -o rez.o -Fpe-x86-64

::gcc -c cfg.c -o cfg.o %CFLAGS%
::gcc -c dlg.c -o dlg.o %CFLAGS%
::gcc -c sub.c -o sub.o %CFLAGS%

gcc zdl.c rez.o cfg.c dlg.c sub.c -o zdl64.exe %CFLAGS% -s -mwindows -flto -Wl,--disable-reloc-section -Wl,--disable-runtime-pseudo-reloc -Wl,-nxcompat -Wl,--no-seh -Wl,-s,-dynamicbase

@set CFLAGS=
