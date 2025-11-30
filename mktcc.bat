windres zdl.rc -o rez.o -Fpe-i386

tcc zdl.c cfg.c dlg.c sub.c rez.o -ozdlt.exe -lcomdlg32 -ladvapi32 -lshell32 -municode -D_UNICODE -DUNICODE
