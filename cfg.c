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
* cfg.c : Config file loading and saving stuff
**********************************************************************/
#include "zdl.h"

  ///////////////////////////////////////////////////////////////////
 // ReadINI - Reads an INI value to a buffer from a given file
//  Written by BioHazard on November 18th 2005 -- Version 1.3
//// USAGE //////////////////////////////////////////////////////////
// section : The INI section to start reading at (don't include []'s)
// entry   : The entry to read to [out] (don't include the '=')
// out     : Buffer to read the line at [entry] to
// limit   : Size of [out] in chars (overflowing output is truncated)
// ini     : Path to the file to be opened
//// RETURN /////////////////////////////////////////////////////////
// >=0 : Number of chars copied to [out]
//  -1 : [ini] could not be opened for reading
//  -2 : [section] could not be found
//  -3 : [entry] could not be found under [section]
//// REMARKS ////////////////////////////////////////////////////////
// [entry] must not start with '['
// <stdio.h> is required (and perhaps <string.h>)
int ReadINI(const TCHAR *section, const TCHAR *entry, TCHAR *out, DWORD limit, const TCHAR *ini)
{
    out[0] = TEXT('\0');
    return GetPrivateProfileString(section, entry, NULL, out, limit, ini) && out[0] != TEXT('\0') ;
}

DWORD mySHDeleteKey(HKEY hkey, LPCTSTR  pszSubKey)
{
    typedef DWORD (WINAPI *SHDeleteKey_fn)(HKEY hkey, LPCTSTR  pszSubKey);
    static SHDeleteKey_fn funk = (SHDeleteKey_fn)1;
    if (funk == (SHDeleteKey_fn)1) {
        HINSTANCE h = LoadLibrary(TEXT("SHLWAPI.DLL"));
        if (h) {
            #ifdef UNICODE
            funk = (SHDeleteKey_fn)GetProcAddress(h, "SHDeleteKeyW");
            #else
            funk = (SHDeleteKey_fn)GetProcAddress(h, "SHDeleteKeyA");
            #endif
            if (!funk)
                FreeLibrary(h);
        }
    }
    if (funk)
        return funk(hkey, pszSubKey);

    return 0;
}

  ///////////////////////////////////////////////////////////////////
 // RegisterFileType - Registers a filetype with Windows
//  Written by BioHazard on November 30th 2005 -- Version 1.0
//// USAGE //////////////////////////////////////////////////////////
// ext      : The extension to register
// type     : The data type to associate with
// nicetype : Description of the data type
// exe      : Executable to run the files with
// command  : command line to stick after [exe]
// icon     : 0-based index of icon in [exe] to be uesd for files
//// REMARKS ////////////////////////////////////////////////////////
// Testen to work in WinXP-Pro and Win2000
void RegisterFileType(TCHAR *ext, TCHAR *type,TCHAR *nicetype,TCHAR *exe,TCHAR* command,int icon)
{
    int i;
    TCHAR *tmp,*tmp2;

    i = lstrlen(command) + lstrlen(exe) + MAX_PATH;
    tmp = calloc(i, sizeof(TCHAR));
    tmp2 = calloc(i, sizeof(TCHAR));

    // Delete the old extensions
    wsprintf(tmp,TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s"),ext);
    SHDeleteKey(HKEY_CURRENT_USER, tmp);
    SHDeleteKey(HKEY_CLASSES_ROOT, ext);
    SHDeleteKey(HKEY_CLASSES_ROOT, type);

    // Add new keys
    RegSetValue(HKEY_CLASSES_ROOT, ext, REG_SZ, type, lstrlen(type));
    RegSetValue(HKEY_CLASSES_ROOT, type,REG_SZ, nicetype, lstrlen(nicetype));
    wsprintf(tmp,  TEXT("%s\\DefaultIcon"), type);
    wsprintf(tmp2, TEXT("%s,%d"), g_pgmptr, icon);
    RegSetValue(HKEY_CLASSES_ROOT,tmp,REG_SZ,tmp2,lstrlen(tmp2));
    wsprintf(tmp, TEXT("%s\\shell\\open\\command"), type);
    wsprintf(tmp2, TEXT("\"%s\" %s"), exe, command);
    RegSetValue(HKEY_CLASSES_ROOT,tmp,REG_SZ,tmp2,lstrlen(tmp2));

    // Free crap
    free(tmp);free(tmp2);
}

 /////////////////////////////////////////////////
// Cfg_LoadConfig : Loads data from the INI file
void Cfg_LoadConfig()
{
    int i=0;
    TCHAR tmp[32]={0};
    ReadINI(TEXT("zdl.general"), TEXT("alwaysadd"), cfg.always, countof(cfg.always), cfg.ini);

    ReadINI(TEXT("zdl.general"), TEXT("zdllaunch"), tmp,countof(tmp),cfg.ini);
    cfg.launch = tmp[0] == '1';
    ReadINI(TEXT("zdl.general"), TEXT("autoclose"), tmp, sizeof(tmp), cfg.ini);
    cfg.autoclose = tmp[0]=='1';

    // Load ports
    for (i=0; i<MAX_ITEM; i++) {
        port[i] = calloc(sizeof(ITEM), 1);
        wsprintf(tmp, TEXT("p%dn"),i);
        if (!ReadINI(TEXT("zdl.ports"), tmp, port[i]->name, MAX_NAME, cfg.ini))
            { free(port[i]); port[i]=0; break; }
        wsprintf(tmp, TEXT("p%df"), i);
        if (!ReadINI(TEXT("zdl.ports"), tmp, port[i]->path, MAX_PATH, cfg.ini))
            { free(port[i]); port[i]=0; break; }

        port[i]->avail = FileExists(port[i]->path);
    }
    // Load IWADs
    iwad[0] = calloc(sizeof(ITEM), 1);
    lstrcpy_s(iwad[0]->name, MAX_NAME, TEXT("None/Auto"));
    //lstrcpy_s(iwad[0]->path, MAX_PATH, TEXT(""));
    iwad[0]->avail = 1;
    
    for (i=1; i<MAX_ITEM; i++) {
        iwad[i] = calloc(sizeof(ITEM), 1);
        wsprintf(tmp, TEXT("i%dn"),i-1);
        if(!ReadINI(TEXT("zdl.iwads"),tmp, iwad[i]->name, MAX_NAME, cfg.ini))
            {free(iwad[i]);iwad[i]=0;break;}
        wsprintf(tmp, TEXT("i%df"),i-1);
        if(!ReadINI(TEXT("zdl.iwads"),tmp, iwad[i]->path, MAX_PATH, cfg.ini))
            {free(iwad[i]);iwad[i]=0;break;}

        // MessageBox(NULL, iwad[i]->path, NULL, 0);
        iwad[i]->avail = FileExists(iwad[i]->path);
    }
}

 /////////////////////////////////////////
// Cfg_GetSel : Gets the Port/IWAD index
int Cfg_GetSel(int sel, ITEM **item)
{
    return 0<= sel && sel < MAX_ITEM? sel: 0;
}

 /////////////////////////////////////////////////
// Cfg_GetSelStr : Gets a selection index from a name
int Cfg_GetSelStr(TCHAR *name,ITEM **item)
{
    int i=0;
    for (i=0; i<MAX_ITEM && item[i]; i++) { // Get to the real index
        if (!lstrcmpi(name, item[i]->name))
            return i;
    }
    return -1;
}


static int lstr2int(const TCHAR *s)
{
    long int v=0;
    int sign=1;
    while (*s == ' ') s++; /*  ||  (unsigned int)(*s - 9) < 5u */

    switch (*s) {
    case '-': sign=-1; /* fall through */
    case '+': ++s;
    }
    while ((unsigned)(*s - '0') < 10u) {
        v = v * 10 + (*s - '0');
        ++s;
    }
    return sign*v;
}
 /////////////////////////////////////////////////
// Cfg_ReadSave : Reads a save file and applies it's settings
int Cfg_ReadSave(HWND dlg, const TCHAR *file)
{
    int i=0,q=0;
    TCHAR tmp[MAX_PATH], tmp2[MAX_PATH];

    static const int boxidEDT[6] = { LST_WARP,EDT_EXTRA,EDT_HOST,EDT_FRAGS,EDT_DMF,EDT_DMF2 };
    static const TCHAR *boxEDT[6]={ TEXT("warp"),TEXT("extra"), TEXT("host"), TEXT("fraglimit"), TEXT("dmflags"), TEXT("dmflags2") };
    static const int boxidLST[3] = { LST_SKILL,LST_GAME,LST_PLAYERS };
    static const TCHAR *boxLST[3] = { TEXT("skill"), TEXT("gametype"), TEXT("players") };

//    if(Cfg_GetSel(0, port)== -1  || Cfg_GetSel(0,iwad) == -1){
//        MessageBox(dlg, STR_NOITEMS, TEXT("Error!"), MB_OK|MB_ICONEXCLAMATION);
//        return 0;
//    }

    memset(tmp, 0, sizeof(tmp));
    memset(tmp2, 0, sizeof(tmp2));
// Try to find [zdl.save] to see if this is a save file
    if(ReadINI(TEXT("zdl.save"),TEXT("keyb"),tmp,MAX_PATH,file)==-2){
        MessageBox(dlg,TEXT("This save file is an invalid format."),TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION);
        i=0;goto exit;
    }
// Start reading the saved data
    // Port
    if (ReadINI(TEXT("zdl.save"),TEXT("port"),tmp,MAX_PATH,file)) {
        if (!port[0] || (i=Cfg_GetSelStr(tmp,port))<0) {
            wsprintf(tmp2, STR_NEEDPRT,tmp,port[Cfg_GetSel(0,port)]->name);
            if(MessageBox(dlg,tmp2,TEXT("Port not found!"),MB_YESNO|MB_ICONQUESTION)==IDNO){i=0;goto exit;}
            i=0;
        }
    }
    // Iwad
    if (ReadINI(TEXT("zdl.save"),TEXT("iwad"),tmp,MAX_PATH,file)) {
        if (!iwad[0] || (q=Cfg_GetSelStr(tmp,iwad))<0) {
            wsprintf(tmp2, STR_NEEDIWD,tmp,iwad[Cfg_GetSel(0,iwad)]->name);
            if(MessageBox(dlg,tmp2,TEXT("Load Failed!"),MB_YESNO|MB_ICONQUESTION)==IDNO){i=0;goto exit;}
            q=0;
        }
    }
    Dlg_ClearAll(dlg);
    SendDlgItemMessage(dlg, LST_PORT, CB_SETCURSEL, i, 0);
    SendDlgItemMessage(dlg, LST_IWAD, LB_SETCURSEL, q, 0);
    Dlg_PopulateWarp(dlg,iwad[Cfg_GetSel(q,iwad)]->path);

    // EditBoxes (Warp, Extra, Host, Frags, DMF, DMF2)
    for(i=0; i < 6; i++) {
        if(ReadINI(TEXT("zdl.save"), boxEDT[i], tmp, MAX_PATH, file)>0) {
            SendDlgItemMessage(dlg, boxidEDT[i], WM_SETTEXT,0,(LPARAM)tmp);
        }
    }
    // ComboBoxes (Skill, GameType, Players)
    for(i=0; i < 3; i++) {
        if(ReadINI(TEXT("zdl.save"), boxLST[i], tmp, MAX_PATH, file) > 0) {
            SendDlgItemMessage(dlg, boxidLST[i], CB_SETCURSEL, (WPARAM)lstr2int(tmp), 0);
        }
    }
    // Dialog Mode
    if (ReadINI(TEXT("zdl.save"), TEXT("dlgmode"), tmp, MAX_PATH, file) > 0
    && !lstrcmpi(tmp,TEXT("open")) && cfg.dlgmode) {
        SendMessage(dlg, WM_COMMAND, MAKELONG(BTN_PANEL, BN_CLICKED), 0);
    }
    // PWAD list
    for(i=0; i < MAX_PWAD ;i++) {
        wsprintf(tmp2, TEXT("file%d"), i);
        if (ReadINI(TEXT("zdl.save"),tmp2,tmp, MAX_PATH,file)>0) {
            if(!Dlg_AddPWAD(dlg,tmp)){i--;}
        } else {
            break;
        }
        if (i >= MAX_PWAD) {
            MessageBox(dlg,STR_MAXPWAD,TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION);
            break;
        }
        q++;
   }
   i=1;
exit: // Free crap and exit

    return i;
}

 /////////////////////////////////////////////////
// Cfg_WriteSave : Writes dialog data to the file given
void Cfg_WriteSave(HWND dlg, const TCHAR *inipath)
{
    // Used for the box grabber
    static const int boxid[] = { EDT_EXTRA, EDT_HOST, EDT_FRAGS, EDT_DMF, EDT_DMF2 };
    static const TCHAR *box[] = {
        TEXT("extra"),
        TEXT("host"),
        TEXT("fraglimit"),
        TEXT("dmflags"),
        TEXT("dmflags2")
    };

    int i=0;
    TCHAR tmp[MAX_PATH];
    tmp[0] = TEXT('\0');

// Standard stuff
    if ((i=SendDlgItemMessage(dlg, LST_PORT, CB_GETCURSEL,0,0)) != LB_ERR) {
        WritePrivateProfileString(TEXT("zdl.save"), TEXT("port"), port[Cfg_GetSel(i,port)]->name, inipath);
    }
    if ((i=SendDlgItemMessage(dlg,LST_IWAD, LB_GETCURSEL,0,0)) != LB_ERR) {
        WritePrivateProfileString(TEXT("zdl.save"), TEXT("iwad"), iwad[Cfg_GetSel(i,iwad)]->name, inipath);
    }

    WritePrivateProfileString(TEXT("zdl.save"), TEXT("dlgmode"), cfg.dlgmode? NULL: TEXT("open"), inipath);

    SendDlgItemMessage(dlg, LST_WARP, WM_GETTEXT, MAX_PATH, (LPARAM)tmp);
    WritePrivateProfileString(TEXT("zdl.save"), TEXT("warp"), tmp, inipath);

    wsprintf(tmp, TEXT("%d"), (int)SendDlgItemMessage(dlg, LST_SKILL, CB_GETCURSEL, 0, 0));
    WritePrivateProfileString(TEXT("zdl.save"), TEXT("skill"), tmp, inipath);

    if((i=SendDlgItemMessage(dlg, LST_GAME, CB_GETCURSEL, 0, 0)) != LB_ERR) {
        wsprintf(tmp, TEXT("%d"), i);
        WritePrivateProfileString(TEXT("zdl.save"), TEXT("gametype"), tmp, inipath);
    }

    if((i=SendDlgItemMessage(dlg, LST_PLAYERS, CB_GETCURSEL, 0, 0)) != LB_ERR) {
        wsprintf(tmp, TEXT("%d"), i);
        WritePrivateProfileString(TEXT("zdl.save"), TEXT("players"), tmp, inipath);
    }
    for (i=0; i < 5; i++) { // Grab text from the boxes
        HWND ithwnd = GetDlgItem(dlg, boxid[i]);
        tmp[0] = TEXT('\0');
        SendMessage(ithwnd, WM_GETTEXT, MAX_PATH, (LPARAM)tmp);
        WritePrivateProfileString(TEXT("zdl.save"), box[i], tmp, inipath);
    }
// PWADs
    for (i=0; i < MAX_PWAD; i++) {
        wsprintf(tmp, TEXT("file%d"), i);
        WritePrivateProfileString(TEXT("zdl.save"), tmp, pwad[i], inipath);
    }
}
