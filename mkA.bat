@del *.o
@del zdla.exe

set CFLAGS=-m32 -Os -std=gnu89 -Wdeclaration-after-statement -mpreferred-stack-boundary=2 -momit-leaf-frame-pointer -nostdlib -march=i386 -lkernel32 -luser32 -lshell32 -lmsvcrt -lcomdlg32 -ladvapi32 -lshlwapi -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -e_MyMain -D__USE_MINGW_ANSI_STDIO=0 -Wall -flto -fno-ident -fno-exceptions -fno-dwarf2-cfi-asm -fmerge-all-constants -Wextra -Wstringop-overflow=4 -Wstack-usage=4096 -Wno-unused-parameter

windres zdl.rc -o rez.o -Fpe-i386

::gcc -c cfg.c -o cfg.o %CFLAGS%
::gcc -c dlg.c -o dlg.o %CFLAGS%
::gcc -c sub.c -o sub.o %CFLAGS%

gcc zdl.c rez.o cfg.c dlg.c sub.c -o zdla.exe %CFLAGS% -s -mwindows -fipa-pta -flto -fwhole-program -flto-partition=none -Wl,--disable-reloc-section -Wl,--disable-runtime-pseudo-reloc -Wl,-nxcompat -Wl,--no-seh -Wl,-s,-dynamicbase

@Copy /B zdla.exe D:\Jeux\PRTS\ZDL\zdl3.exe
@set CFLAGS=
