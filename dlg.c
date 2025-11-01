/**********************************************************************
* ZDL!
* Copyright (C) 2005 BioHazard / Vectec Software
***********************************************************************
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 or any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
***********************************************************************
* dlg.c : Most of the important functions are here
**********************************************************************/
#include "zdl.h"
#define isDigit(x) ( '0' <= (x) && (x) <= '9' )
#define isFileExt(x, Y) ( !lstrcmpi(lstrrchr((x), TEXT('.')), (Y)) )
/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Dlg_Launch : The most important function in ZDL */
void Dlg_Launch(HWND dlg, char prompt)
{
    /* Max commend line length (q) = MAXPATH * (1(exe) + 1(IWAD) + NUM_PWAD + 1extra) */
    size_t q = MAX_PATH*(MAX_PWAD+4), cmdlen;
    int i=0, pw=0, pt=0, pl=0;
    TCHAR *cmd, tmp[MAX_PATH], *portExe;
    /* Make sure there is a port selected */
    if(!SendDlgItemMessage(dlg,LST_PORT,CB_GETCOUNT,0,0)||!SendDlgItemMessage(dlg,LST_IWAD,LB_GETCOUNT,0,0)){MessageBox(dlg,STR_NOITEMS,TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION);return;}
    /* Determine how long the cmd string has to be */
    for(i=0; i<MAX_PWAD && pwad[i]; i++)
        q+=lstrlen(pwad[i])+4; /* Count the length of the PWADs */

    /* Alloc the commend line string (up to 18KB) */
    cmdlen = q;
    cmd = calloc(cmdlen, sizeof(TCHAR));
    if (!cmd) return;
    memset(tmp,0, sizeof(tmp));
    portExe = port[Cfg_GetSel(SendDlgItemMessage(dlg, LST_PORT, CB_GETCURSEL, 0, 0),port)]->path;

    /* Print the basic stuff */
    {
    const TCHAR *iwadpath=iwad[Cfg_GetSel(SendDlgItemMessage(dlg,LST_IWAD, LB_GETCURSEL,0,0),iwad)]->path;
    if (iwadpath && *iwadpath)
        wsprintf(cmd
            ,TEXT("\"%s\" -iwad \"%s\"%s%s")
            , portExe
            , iwadpath
            , (lstrlen(cfg.always))? TEXT(" "): TEXT(""), cfg.always);
    else
        wsprintf(cmd
            ,TEXT("\"%s\" %s%s")
            , portExe
            , (lstrlen(cfg.always))? TEXT(" "): TEXT(""), cfg.always);

    }
    /* Warp and Skill */
    SendDlgItemMessage(dlg,LST_WARP,WM_GETTEXT,MAX_PATH,(LPARAM)tmp);
    if (tmp[0]) {
        TCHAR mapnum[MAX_PATH], *p; /* In case it is a simple map number */
        int issimplenumber = 0;
        mapnum[0] = '\0';
        p = mapnum;


        if(tmp[0] == 'M'
        && tmp[1] == 'A'
        && tmp[2] == 'P'
        && isDigit(tmp[3])
        && isDigit(tmp[4])
        && !tmp[5] ) {
            /* "MAPXY" format */
            p[0] = tmp[3];
            p[1] = tmp[4];
            p[2] = '\0';
            issimplenumber = 1;
        } else if( tmp[0] == 'E' && isDigit(tmp[1])
               && tmp[2] == 'M' && isDigit(tmp[3]) && !tmp[4] ) {
            /* EXMY format */
            p[0] = tmp[1];
            p[1] = ' ';
            p[2] = tmp[3];
            p[3] = '\0';
            issimplenumber = 1;
        } else {
            p = tmp;
            /* "XY" or 'X Y" */
            issimplenumber = ( isDigit(p[0]) && isDigit(p[1]) && !p[2] )
                          || ( isDigit(p[0]) && p[1] == TEXT(' ') && isDigit(p[2]) && !p[3] );
        }

        /* We use -warp XX if possible otherwise we use ZDoom's +map */
        if(*p) {
            lstrcat_s(cmd, cmdlen, issimplenumber? TEXT(" -warp "): TEXT(" +map "));
            lstrcat_s(cmd, cmdlen, p);
        }

        /* Skill */
        wsprintf(
            &cmd[lstrlen(cmd)]
            , TEXT(" -skill %d"),(int)SendDlgItemMessage(dlg, LST_SKILL, CB_GETCURSEL,0,0)+1);
    }

    /* PWADs and DEHs */
    for (i=0; i < MAX_PWAD && pwad[i]; i++) { /* Count up each type of file */
        if(isFileExt(pwad[i], TEXT(".deh"))
        || isFileExt(pwad[i], TEXT(".bex"))) {
            /* DEH/BEX */
            pt++;
        } else if (isFileExt(pwad[i], TEXT(".lmp"))) {
            /* LMP (DEMO file) */
            pl++;
        } else {
            /* Default to wad file. */
            pw++;
        }
    }

    if (pt) { /* Append DEHs */
        lstrcat_s(cmd, cmdlen, TEXT(" -deh"));
        for(i=0; i<MAX_PWAD && pwad[i]; i++) {
            if(isFileExt(pwad[i], TEXT(".deh"))
            || isFileExt(pwad[i], TEXT(".bex"))) {
                wsprintf(&cmd[lstrlen(cmd)], TEXT(" \"%s\""), pwad[i]);
            }
        }
    }
    if (pl == 1) { /* Append DEMO file (if a single one)! */
        lstrcat_s(cmd, cmdlen, TEXT(" -playdemo"));
        for(i=0; i<MAX_PWAD && pwad[i]; i++) {
            if(isFileExt(pwad[i], TEXT(".lmp"))) {
                wsprintf(&cmd[lstrlen(cmd)], TEXT(" \"%s\""), pwad[i]);
            }
        }
    }
    if (pw) { /* Append PWADs */
        lstrcat_s(cmd, cmdlen, TEXT(" -file"));
        for(i=0; i<MAX_PWAD && pwad[i]; i++){
            if(!isFileExt(pwad[i], TEXT(".deh"))
            && !isFileExt(pwad[i], TEXT(".bex"))
            && (pl!=1 || !isFileExt(pwad[i], TEXT(".lmp"))) ) {
                wsprintf(&cmd[lstrlen(cmd)], TEXT(" \"%s\""), pwad[i]);
            }
        }
    }

    /* Extra Args */
    if(SendDlgItemMessage(dlg, EDT_EXTRA,WM_GETTEXTLENGTH,0,0)){
        lstrcat_s(cmd, cmdlen, TEXT(" "));
        SendDlgItemMessage(dlg,EDT_EXTRA,WM_GETTEXT,q-lstrlen(cmd),(LPARAM)&cmd[lstrlen(cmd)]);
    }
    /* Network stuff */
    if(SendDlgItemMessage(dlg,LST_GAME,CB_GETCURSEL,0,0)){
        if((i=SendDlgItemMessage(dlg,LST_PLAYERS,CB_GETCURSEL,0,0))){ /* Hosting */
            wsprintf(&cmd[lstrlen(cmd)], TEXT(" -host %d"),i);
            /* Deathmatch Flag */
            if(SendDlgItemMessage(dlg,LST_GAME,CB_GETCURSEL,0,0)==2){
                lstrcat_s(cmd, cmdlen, TEXT(" -deathmatch"));
                /* Fragimit */
                if(SendDlgItemMessage(dlg,EDT_FRAGS,WM_GETTEXTLENGTH,0,0)){
                    lstrcat_s(cmd, cmdlen, TEXT(" +fraglimit "));
                    SendDlgItemMessage(dlg,EDT_FRAGS,WM_GETTEXT,q-lstrlen(cmd),(LPARAM)&cmd[lstrlen(cmd)]);
                }
            }
            /* DMF */
            if(SendDlgItemMessage(dlg,EDT_DMF,WM_GETTEXTLENGTH,0,0)){
                lstrcat_s(cmd, cmdlen, TEXT(" +dmflags "));
                SendDlgItemMessage(dlg,EDT_DMF,WM_GETTEXT,q-lstrlen(cmd),(LPARAM)&cmd[lstrlen(cmd)]);
            }
            /* DMF2 */
            if(SendDlgItemMessage(dlg,EDT_DMF2,WM_GETTEXTLENGTH,0,0)){
                lstrcat_s(cmd, cmdlen, TEXT(" +dmflags2 "));
                SendDlgItemMessage(dlg,EDT_DMF2,WM_GETTEXT,q-lstrlen(cmd),(LPARAM)&cmd[lstrlen(cmd)]);
            }
        }else{ /* Joining */
            if(!SendDlgItemMessage(dlg,EDT_HOST,WM_GETTEXTLENGTH,0,0)) {
                MessageBox(dlg, TEXT("You need to type a host to join"), TEXT("Error!"), MB_OK|MB_ICONHAND);
                goto exit;
            }
            lstrcat_s(cmd, cmdlen, TEXT(" -join "));
            SendDlgItemMessage(dlg, EDT_HOST,WM_GETTEXT, q-lstrlen(cmd), (LPARAM)&cmd[lstrlen(cmd)]);
        }
    }
    if(prompt&&MessageBox(dlg,cmd,TEXT("Run this command-line?"),MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)==IDNO){goto exit;}

    {/* Change to the port directory */
        TCHAR *end;
        lstrcpy_s(tmp, countof(tmp), portExe);
        end = lstrrchr(tmp, '\\');
        if (end) {
            *end = TEXT('\0');
            /* MessageBox(NULL,tmp,TEXT("Going in directory"), MB_OK); */
            SetCurrentDirectory(tmp);
        }
    }

    { /* GoooOOooooOOOOOoooOOO! */
    STARTUPINFO lpStartupInfo;
    PROCESS_INFORMATION lpProcessInformation;
    memset(&lpProcessInformation, 0, sizeof(lpProcessInformation));
    memset(&lpStartupInfo, 0, sizeof(lpStartupInfo));
    lpStartupInfo.cb = sizeof(STARTUPINFO);
    CreateProcess(
        NULL, /* lpApplicationName */
        cmd,  /* lpCommandLine */
        NULL, NULL, /* lpProcessAttributes, lpThreadAttributes */
        FALSE, /* bInheritHandles */
        0, /* dwCreationFlags */
        NULL, /* lpEnvironment */
        NULL, /* lpCurrentDirectory */
        &lpStartupInfo, /* lpStartupInfo */
        &lpProcessInformation /* lpProcessInformation */
    );
    }
    /* Quit on launch? */
    if (cfg.autoclose) {
        Dlg_Quit(dlg, 1);
    }
exit:
    free(cmd);
}

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *  Dlg_AddPWAD : Adds an item to the PWAD list  */
int Dlg_AddPWAD(HWND dlg, const TCHAR *file)
{
    HWND ithwnd = GetDlgItem(dlg,LST_PWAD);
    int i=SendMessage(ithwnd, LB_GETCOUNT, 0, 0);

    if (i >= MAX_PWAD
    || !lstrrchr(file, TEXT('.'))
    || isFileExt(file, TEXT(".zdl"))
    || isFileExt(file, TEXT(".ini"))) {
        return 0;
    }

    pwad[i] = calloc(MAX_PATH, sizeof(TCHAR));
    lstrcpy_s(pwad[i], MAX_PATH, file);
    SendMessage(ithwnd, LB_ADDSTRING, 0, (LPARAM)GetFNinPath(file));

    return 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Dlg_ClearPWAD : Clears all the data from the PWAD list  */
void Dlg_ClearPWAD(HWND dlg){
    int i=0;
    SendDlgItemMessage(dlg, LST_PWAD, LB_RESETCONTENT, 0, 0);
    for(i=0; i<MAX_PWAD && pwad[i]; i++) {
        free(pwad[i]);
        pwad[i] = NULL;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Dlg_ClearAll : Clears all the data from the dialog  */
void Dlg_ClearAll(HWND dlg)
{
    int i=0;
    static const int boxL[3] = { LST_PORT, LST_GAME, LST_PLAYERS};
    static const int boxE[5] = { EDT_EXTRA, EDT_HOST, EDT_FRAGS, EDT_DMF, EDT_DMF2 };
    Dlg_ClearPWAD(dlg);
    SendDlgItemMessage(dlg, LST_IWAD,  LB_SETCURSEL, 0, 0);
    SendDlgItemMessage(dlg, LST_WARP,  WM_SETTEXT, 0, (LPARAM)TEXT("") ); /* NONE */
    SendDlgItemMessage(dlg, LST_SKILL, CB_SETCURSEL, 2, 0);
    for(i=0; i<3; i++){ SendDlgItemMessage(dlg, boxL[i], CB_SETCURSEL, 0, 0); }
    for(i=0; i<5; i++){ SendDlgItemMessage(dlg, boxE[i], WM_SETTEXT, 0, (LPARAM)TEXT("")); }
}

/* * * * * * * * * * * * * * * * * * * *
 * Dlg_Quit : Saves data and exits ZDL */
void Dlg_Quit(HWND dlg, char save)
{
    int i=0;
    TCHAR tmp[64];

    /* Save the config */
    if (save) {
        WritePrivateProfileString(TEXT("zdl.general"), TEXT("alwaysadd"), cfg.always, cfg.ini);
        WritePrivateProfileString(TEXT("zdl.general"), TEXT("zdllaunch"), cfg.launch? TEXT("1"): NULL, cfg.ini);
        WritePrivateProfileString(TEXT("zdl.general"), TEXT("autoclose"), cfg.autoclose? TEXT("1"): NULL, cfg.ini);

        /* Write ports */
        for (i=0; i < MAX_ITEM; i++) {
            wsprintf(tmp, TEXT("p%dn"), i);
            WritePrivateProfileString(TEXT("zdl.ports"), tmp, port[i]? port[i]->name: NULL, cfg.ini);

            wsprintf(tmp, TEXT("p%df"), i);
            WritePrivateProfileString(TEXT("zdl.ports"), tmp, port[i]? port[i]->path: NULL, cfg.ini);
        }
        /* Write IWADs */
        for(i=1; i < MAX_ITEM; i++) {
            wsprintf(tmp, TEXT("i%dn"), i-1);
            WritePrivateProfileString(TEXT("zdl.iwads"), tmp, iwad[i]? iwad[i]->name: NULL, cfg.ini);

            wsprintf(tmp, TEXT("i%df"), i-1);
            WritePrivateProfileString(TEXT("zdl.iwads"), tmp, iwad[i]? iwad[i]->path: NULL, cfg.ini);
        }

        Cfg_WriteSave(dlg, cfg.ini);
    }
    /* Delete port and IWAD data */
    for (i=0; i<MAX_ITEM && port[i]; i++){ free(port[i]); }
    for (i=0; i<MAX_ITEM && iwad[i]; i++){ free(iwad[i]); }
    Dlg_ClearPWAD(dlg);
    EndDialog(dlg,0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * PopulateWarp : Reads through the wad and finds valid maps */
void Dlg_PopulateWarp(HWND dlg, TCHAR *file)
{
    /* * * * * *
     * From DoomDefs.h (by Bio) */
    typedef struct {
        char type[4];   /* Either "IWAD" or "PWAD" */
        DWORD lumps;    /* Number of lumps in the dir */
        DWORD dir;      /* the offset of the start of the dir */
    } WADHEAD;
    typedef struct {
        DWORD start;    /* where the lump starts */
        DWORD length;   /* how long the lump is */
        char name[8];   /* the lump's name (e.g. "MAP01") */
    } LUMPHEAD;


    unsigned i=0;
    DWORD read=0;
    char temp[9]= { 0 };
    WADHEAD header;
    LUMPHEAD lump[2];
    HANDLE fptr;
    HWND ithwnd;
    BOOL ok;

    ithwnd = GetDlgItem(dlg,LST_WARP);

    SendMessageA(ithwnd, CB_RESETCONTENT,0,0);
    SendMessageA(ithwnd, CB_ADDSTRING, 0, (LPARAM)""); /* NONE */

    /* MessageBox(NULL, file, TEXT("Read Map Names in:"), 0); */
    if(!file || !FileExists(file))
        return;

    fptr = CreateFile(
        file, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL
    );

    if (fptr==INVALID_HANDLE_VALUE) { MessageBox(dlg, TEXT("Unable to Open IWAD file to read maps"), NULL, MB_OK); return; }

    ok = ReadFile(fptr, &header, sizeof(WADHEAD), &read, NULL);
    if (!ok) { MessageBox(dlg, TEXT("Unable to Read IWAD file!"), NULL, MB_OK); return; }
    SetFilePointer(fptr, header.dir, 0, FILE_BEGIN);

    for (i=0; i < header.lumps; i++) {
        ok = ReadFile(fptr, &lump, sizeof(LUMPHEAD)*2, &read, NULL);
        if (!ok) break;
        if ( lump[1].name[0] == 'T' && !lstrcmpiA(lump[1].name, "THINGS")) {
            lstrcpynA(temp, lump[0].name, 8);
            SendMessageA(ithwnd, CB_ADDSTRING, 0, (LPARAM)temp);
        } else {
            SetFilePointer(fptr, sizeof(LUMPHEAD)-sizeof(LUMPHEAD)*2, 0, FILE_CURRENT);
        }
    }
    SendMessageA(ithwnd, CB_SETCURSEL,0,0);
    CloseHandle(fptr);
}
