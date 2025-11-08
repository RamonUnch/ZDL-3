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
* zdl.c : Entry point, System vars and Window Procedures
**********************************************************************/
#include "zdl.h"

void lstrcpy_sA(char *__restrict__ dest, size_t N, const char *src)
{
    char *dmax=dest+N-1; /* keep space for a terminating NULL */
    for (; dest<dmax && (*dest=*src); ++src,++dest);  /* then append from src */
    *dest='\0'; /* ensure result is NULL terminated */
}

void lstrcpy_sW(wchar_t *__restrict__ dest, size_t N, const wchar_t *src)
{
    wchar_t *dmax=dest+N-1; /* keep space for a terminating NULL */
    for (; dest<dmax && (*dest=*src); ++src,++dest);  /* then append from src */
    *dest='\0'; /* ensure result is NULL terminated */
}

char *lstrcat_sA(char *__restrict__ d, const size_t N, const char *__restrict__ s)
{
    const char *dmax = d + N - 1; /* keep space for a terminating NULL */
    for (; d<dmax &&  *d ; ++d);             /* go to end of dest */
    for (; d<dmax && (*d=*s); ++d,++s);  /* then append from src */
    *d = '\0'; /* ensure result is NULL terminated */
    return d;
}

wchar_t *lstrcat_sW(wchar_t *__restrict__ d, const size_t N, const wchar_t *__restrict__ s)
{
    const wchar_t *dmax = d + N - 1; /* keep space for a terminating NULL */
    for (; d<dmax &&  *d ; ++d);             /* go to end of dest */
    for (; d<dmax && (*d=*s); ++s,++d);  /* then append from src */
    *d = '\0'; /* ensure result is NULL terminated */
    return d;
}

BOOL GetOpenFileNameCompat(OPENFILENAME *ofn)
{
    BOOL ret;
    ret = GetOpenFileName(ofn);
#ifndef _WIN64
    if (!ret && CommDlgExtendedError() == CDERR_STRUCTSIZE) {
        ofn->lStructSize = 19 * 4; /* Old Win9x / NT4 size */
        ret = GetOpenFileName(ofn);
    }
#endif
    return ret;
}
BOOL GetSaveFileNameCompat(OPENFILENAME *ofn)
{
    BOOL ret;
    ret = GetSaveFileName(ofn);
#ifndef _WIN64
    if (!ret && CommDlgExtendedError() == CDERR_STRUCTSIZE) {
        ofn->lStructSize = 19 * 4; /* Old Win9x / NT4 size */
        ret = GetSaveFileName(ofn);
    }
#endif
    return ret;
}

/* Global data */
CFG cfg={0};
ITEM *iwad[MAX_ITEM+1]={0},*port[MAX_ITEM+1]={0};
TCHAR *pwad[MAX_PWAD+1]={0};
TCHAR *cmdline=NULL;
short arg1=0,arg2=0,arg3=0; /* Needed for the config dialogs */

static void OnDeleteItem(HWND dlg, int list_id)
{
    int i=0;
    HWND ithwnd;
    ITEM **item;

    item = list_id == LST_PORT ? (port) : (iwad);
    ithwnd = GetDlgItem(dlg, list_id);
    if ((i=SendMessage(ithwnd,LB_GETCURSEL,0,0)) != LB_ERR) {
        free(item[i]);
        SendMessage(ithwnd, LB_DELETESTRING, i, 0);
        SendMessage(ithwnd, LB_SETCURSEL, i, 0);
        for( ; i < MAX_ITEM-1 && item[i]; i++)
            item[i] = item[i+1];
    }
}

static void OnMoveUpOrDown(HWND dlg, int list_id, BOOL up)
{
    int i=0;
    HWND ithwnd;
    ITEM *tmpi;
    ITEM **item;

    item = list_id == LST_PORT ? (port) : (iwad);
    ithwnd = GetDlgItem(dlg, list_id);
    if (up) {
        /* MOVING UP! */
        if((i=SendMessage(ithwnd, LB_GETCURSEL, 0, 0)) != LB_ERR && i!=0) {
            /* Move the entry in the window */
            SendMessage(ithwnd, LB_INSERTSTRING, i-1, (LPARAM)item[i]->name);
            SendMessage(ithwnd, LB_DELETESTRING, i+1, 0);
            SendMessage(ithwnd, LB_SETCURSEL,    i-1, 0);
            /* Move the item */
            tmpi=item[i-1]; item[i-1]=item[i]; item[i]=tmpi;
        }
    } else {
        /* Moving down! */
        if((i=SendMessage(ithwnd, LB_GETCURSEL, 0, 0)) != LB_ERR && i != SendMessage(ithwnd,LB_GETCOUNT,0,0)-1) {
            /* Move the entry in the window */
            SendMessage(ithwnd, LB_INSERTSTRING, i+2, (LPARAM)item[i]->name);
            SendMessage(ithwnd, LB_DELETESTRING, i,   0);
            SendMessage(ithwnd, LB_SETCURSEL,    i+1, 0);
            /* Move the item */
            tmpi=item[i+1]; item[i+1]=item[i]; item[i]=tmpi;
        }
    }
}

INT_PTR CALLBACK ConfigProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int i=0, q=0;
    switch(msg) {
    case WM_INITDIALOG:{
        /* Set icons */
        HMODULE hm = GetModuleHandle(NULL);
        SendDlgItemMessage(dlg,BTN_UP,  BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_UP)));
        SendDlgItemMessage(dlg,BTN_DWN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_DOWN)));
        SendDlgItemMessage(dlg,BTN_IUP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_UP)));
        SendDlgItemMessage(dlg,BTN_IDWN,BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_DOWN)));

        /* Set text limits */
        SendDlgItemMessage(dlg, EDT_EXTRA, EM_LIMITTEXT, countof(cfg.always), 0);

        /* Fill the dialog with info */
        SendDlgItemMessage(dlg, EDT_EXTRA,    WM_SETTEXT, 0, (LPARAM)cfg.always);
        SendDlgItemMessage(dlg, CHK_LAUNCH,   BM_SETCHECK, cfg.launch,   0);
        SendDlgItemMessage(dlg, CHK_AUTOCLOSE,BM_SETCHECK, cfg.autoclose,0);

        /* Fill Port and IWAD lists */
        for (i=0; i<MAX_ITEM && port[i] && port[i]->name[0]; i++) {
            SendDlgItemMessage(dlg, LST_PORT, LB_ADDSTRING, 0, (LPARAM)port[i]->name);
        }
        /*SendDlgItemMessage(dlg, LST_IWAD, LB_ADDSTRING, 0, (LPARAM)TEXT("None"));*/
        for (i=0; i<MAX_ITEM && iwad[i] && iwad[i]->name[0]; i++) {
            SendDlgItemMessage(dlg, LST_IWAD, LB_ADDSTRING, 0, (LPARAM)iwad[i]->name);
        }
    }break;
    case WM_COMMAND:switch(HIWORD(wp)){
        case LBN_DBLCLK:
            switch(LOWORD(wp)) {
            case LST_PORT:
            case LST_IWAD: {
                ITEM **item;
                HWND ithwnd;
                arg1 = q = LOWORD(wp);
                arg2 = i = (short)SendDlgItemMessage(dlg, LOWORD(wp), LB_GETCURSEL, 0, 0);
                item = (LOWORD(wp)==LST_PORT)? (port): (iwad);
                DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_FILE),dlg,FileProc);
                /* Update the string in the listbox */
                ithwnd = GetDlgItem(dlg,q);
                SendMessage(ithwnd, LB_DELETESTRING,i,0);
                SendMessage(ithwnd, LB_INSERTSTRING,i,(LPARAM)item[i]->name);
                SendMessage(ithwnd, LB_SETCURSEL,q,0);
                /* Update the file's availabillity */
                item[i]->avail = FileExists(item[i]->path);
                arg1=arg2=0; /* Reset the temp vars */
            }break;
        }break;
        case BN_CLICKED:
            switch(LOWORD(wp)) {
            case BTN_ZDL:{
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_ASSOC), dlg, AssocProc);
            }break;
            case BTN_ADD:
            case BTN_IADD: {
                HWND ithwnd;
                ITEM **item = (LOWORD(wp)==BTN_ADD)? (port): (iwad);
                arg1 = q = (LOWORD(wp)==BTN_ADD)? (LST_PORT): (LST_IWAD);
                arg2 = -1; /* -1 == BTN_ADD pressed */
                ithwnd = GetDlgItem(dlg,q);
                arg3 = i = (short)SendMessage(ithwnd, LB_GETCOUNT, 0, 0);
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_FILE), dlg, FileProc);
                /* If a new entry was added, add it to the listbox */
                if (item[i]) {
                    SendMessage(ithwnd, LB_ADDSTRING, 0, (LPARAM)item[i]->name);
                }
                arg1=arg2=arg3=0; /* Reset the temp vars */
            }break;
            /* Delete button on PORT and IWAD lists */
            case BTN_REM:  OnDeleteItem(dlg, LST_PORT); break;
            case BTN_IREM: OnDeleteItem(dlg, LST_IWAD); break;


            /* Up and down buttons for PORT and IWAD lists */
            case BTN_UP:   OnMoveUpOrDown(dlg, LST_PORT, /*UP=*/ TRUE ); break;
            case BTN_IUP:  OnMoveUpOrDown(dlg, LST_IWAD, /*UP=*/ TRUE ); break;
            case BTN_DWN:  OnMoveUpOrDown(dlg, LST_PORT, /*UP=*/ FALSE); break;
            case BTN_IDWN: OnMoveUpOrDown(dlg, LST_IWAD, /*UP=*/ FALSE); break;

            case IDOK:
                SendDlgItemMessage(dlg, EDT_EXTRA, WM_GETTEXT, countof(cfg.always), (LPARAM)cfg.always);
                cfg.launch = (char)SendDlgItemMessage(dlg, CHK_LAUNCH,   BM_GETCHECK, 0, 0);
                cfg.autoclose=(char)SendDlgItemMessage(dlg,CHK_AUTOCLOSE,BM_GETCHECK, 0, 0);
            /* Fall through */
            case IDCANCEL:
                EndDialog(dlg, 0);
            break;
        }break;
    }break;
    default:;
    }

    return 0;
}

int SendMessageFNb(HWND ithwnd, UINT msg, WPARAM wp, const TCHAR *fn, char exists)
{
    if (exists) {
        return SendMessage(ithwnd, msg, wp, (LPARAM)fn);
    } else {
        /* Prefix with a * if file is not found */
        TCHAR buf[MAX_PATH+2];
        buf[0] = TEXT('*');
        lstrcpy_s(buf+1, countof(buf)-1, fn);
        return SendMessage(ithwnd, msg, wp, (LPARAM)buf);
    }

}

static void OnDeletePWAD(HWND dlg)
{
    int i=0;
    HWND ithwnd = GetDlgItem(dlg,LST_PWAD);
    if((i=SendMessage(ithwnd, LB_GETCURSEL, 0, 0))!=LB_ERR){
        free(pwad[i]);
        SendMessage(ithwnd, LB_DELETESTRING, i, 0);
        SendMessage(ithwnd, LB_SETCURSEL, i, 0);
        for( ; i < MAX_PWAD && pwad[i]; i++) {
            pwad[i] = pwad[i+1];
        }
    }
}

INT_PTR CALLBACK MainProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp)
{
    int i=0,q=0,r=0,m=0;
    RECT rct;

    switch (msg) {
    case WM_INITDIALOG: {
        /* Set icons */
        HWND ithwnd;
        HMODULE hm = GetModuleHandle(NULL);
        SendMessage(dlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_ICON)));
        SendMessage(dlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_ICON)));
        SendDlgItemMessage(dlg, BTN_UP,  BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_UP)));
        SendDlgItemMessage(dlg, BTN_DWN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hm, MAKEINTRESOURCE(ICO_DOWN)));

        /* Insert list data */
        { /* SKILL */
        static const TCHAR *skill_labels[5] = { TEXT("V. Easy"), TEXT("Easy"), TEXT("Normal"), TEXT("Hard") ,TEXT("V. Hard") };
        ithwnd = GetDlgItem(dlg,LST_SKILL);
        for (i = 0; i < (int)countof(skill_labels); i++)
            SendMessage(ithwnd, CB_ADDSTRING, 0, (LPARAM)skill_labels[i]);
        SendMessage(ithwnd, CB_SETCURSEL, 2, 0);
        }
        { /* Game mode */
        static const TCHAR *mode_labels[3] = { TEXT("Single-Player"), TEXT("Multi Co-operative"),TEXT("Multi DeathMatch") };
        ithwnd = GetDlgItem(dlg,LST_GAME);
        for (i = 0; i < (int)countof(mode_labels); i++)
            SendMessage(ithwnd, CB_ADDSTRING, 0, (LPARAM)mode_labels[i]);
        SendMessage(ithwnd, CB_SETCURSEL, 0, 0);
        }
        { /* Number of Players */
        static const TCHAR *nplay_labels[9] = { TEXT("Joining"),TEXT("1"),TEXT("2"),TEXT("3"),TEXT("4"),TEXT("5"),TEXT("6"),TEXT("7"),TEXT("8") };
        ithwnd = GetDlgItem(dlg, LST_PLAYERS);
        for (i = 0; i < (int)countof(nplay_labels) ; i++)
            SendMessage(ithwnd, CB_ADDSTRING, 0, (LPARAM)nplay_labels[i]);
        SendMessage(ithwnd, CB_SETCURSEL, 0, 0);
        }
        /* Set text limits */
        SendDlgItemMessage(dlg, LST_WARP, CB_LIMITTEXT, 8, 0);
        SendDlgItemMessage(dlg, EDT_EXTRA,EM_LIMITTEXT, 128, 0);
        SendDlgItemMessage(dlg, EDT_HOST, EM_LIMITTEXT, 21, 0);
        SendDlgItemMessage(dlg, EDT_FRAGS,EM_LIMITTEXT, 3, 0);
        SendDlgItemMessage(dlg, EDT_DMF,  EM_LIMITTEXT, 8, 0);
        SendDlgItemMessage(dlg, EDT_DMF2, EM_LIMITTEXT, 8, 0);

        /* Fill the port and IWAD arrays */
        if ((i = Cfg_GetSel(0, port)) != -1) {
            ithwnd = GetDlgItem(dlg, LST_PORT);
            for ( ; i<MAX_ITEM && port[i]; i++) {
                SendMessageFNb(ithwnd, CB_ADDSTRING, 0, port[i]->name, port[i]->avail);
            }
            SendMessage(ithwnd,CB_SETCURSEL,0,0);
        }
        if ((i = Cfg_GetSel(0, iwad)) != -1) {
            ithwnd = GetDlgItem(dlg,LST_IWAD);
            /*SendMessageFNb(ithwnd,LB_ADDSTRING,0, TEXT("None"), 1);*/
            for( ; i<MAX_ITEM && iwad[i]; i++){
                SendMessageFNb(ithwnd,LB_ADDSTRING,0, iwad[i]->name, iwad[i]->avail);
            }
            SendMessage(ithwnd, LB_SETCURSEL, 0, 0);
            if ((i = Cfg_GetSel(0,iwad)) != -1 && i < MAX_ITEM && iwad[i]) {
                Dlg_PopulateWarp(dlg, iwad[i]->path);
            }
        }

        /* Load the last configuration */
        if (Cfg_GetSel(0, port) == -1 || Cfg_GetSel(0, iwad) == -1) {
            MessageBox(dlg, STR_NOITEMS, TEXT("Error!"), MB_OK|MB_ICONEXCLAMATION);
            break;
        }
        if (!cmdline || !cmdline[0]) {
            Cfg_ReadSave(dlg, cfg.ini);
            break;
        }

        /* Process the command-line stuff */
        {
        TCHAR tmp[MAX_PATH];
        for ( i = SendDlgItemMessage(dlg, LST_PWAD, LB_GETCOUNT, 0, 0); cmdline[q]; i++) {
            if (i >= MAX_PWAD) {
                MessageBox(dlg, STR_MAXPWAD, TEXT("Error!"), MB_OK|MB_ICONEXCLAMATION);
                break;
            }
            for ( ;cmdline[q]; q++) {
                if (cmdline[q] == '\"') { m=(m)? (0): (1); continue; }
                if (r < MAX_PATH){ tmp[r]=cmdline[q]; r++; }
                if (cmdline[q]==' ' && !m) { q++; break; }
            }
            tmp[(cmdline[q])?(r-1):(r)] = '\0'; r = 0;
            /* Check to see if it's a save file and act accordingly. */
            if (!Dlg_AddPWAD(dlg, tmp)) { /* Check if it's a save file and launch */
                if (i == 0&&lstrchr(tmp, '.')
                && (!lstrcmpi(lstrchr(tmp,'.'), TEXT(".zdl")) || !lstrcmpi(lstrchr(tmp,'.'),TEXT(".ini")))) {
                    if (Cfg_ReadSave(dlg, tmp)) {
                        if (cfg.launch) { /* Launch if the option is set */
                            Dlg_Launch(dlg, 0);
                            Dlg_Quit(dlg, 0);
                        }
                    } else {
                        if (cfg.launch) { Dlg_Quit(dlg,0); }
                    }
                } else { i--; }
            }
        }
        }
    }break;

    case WM_COMMAND:switch(HIWORD(wp)){
        /* File Info */
        case LBN_DBLCLK:switch(LOWORD(wp)){
            case LST_PWAD:MessageBox(dlg,pwad[SendDlgItemMessage(dlg,LST_PWAD,LB_GETCURSEL,0,0)],TEXT("File Info"),MB_OK|MB_ICONINFORMATION);break;
            case LST_IWAD:MessageBox(dlg,iwad[Cfg_GetSel(SendDlgItemMessage(dlg,LST_IWAD,LB_GETCURSEL,0,0),iwad)]->path,TEXT("File Info"),MB_OK|MB_ICONINFORMATION);break;
        }break;
        /* IWAD List */
        case LBN_SELCHANGE:
           if(LOWORD(wp)==LST_IWAD) {
               Dlg_PopulateWarp(dlg, iwad[Cfg_GetSel(SendDlgItemMessage(dlg, LST_IWAD, LB_GETCURSEL, 0, 0), iwad)]->path);
           }
        break;
        /* Buttons */
        case BN_CLICKED:switch(LOWORD(wp)){
            case BTN_LAUNCH:Dlg_Launch(dlg,0);break;
            case MNU_CMD:Dlg_Launch(dlg,1);break;
            case BTN_ZDL: {
                POINT pt;
                GetCursorPos(&pt);
                TrackPopupMenu(GetSubMenu(LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(MNU_MENU)),0),TPM_LEFTALIGN,pt.x,pt.y,0,dlg,0);
            } break;
            /* Loading and Saving */
            case MNU_SAVE:case MNU_LOAD: {
                TCHAR tmpfn[MAX_PATH];
                OPENFILENAME ofn;
                memset(tmpfn, 0, sizeof(tmpfn));
                memset(&ofn, 0, sizeof(ofn));

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = dlg;
                ofn.hInstance = GetModuleHandle(NULL);
                ofn.lpstrFile = tmpfn;
                ofn.nMaxFile  = MAX_PATH;
                ofn.lpstrFilter=TEXT("ZDL Save Files (*.zdl,*.ini)\0*.zdl;*.ini\0All Files (*.*)\0*.*\0");
                ofn.lpstrDefExt=TEXT("zdl");
                ofn.Flags = LOWORD(wp)==MNU_SAVE? OFN_EXPLORER: OFN_FILEMUSTEXIST|OFN_EXPLORER;
                ofn.lpstrTitle = LOWORD(wp)==MNU_SAVE? TEXT("Save ZDL Config"): TEXT("Load a Saved ZDL Config");
                if (LOWORD(wp)==MNU_SAVE) {
                    if (GetSaveFileName(&ofn)) {
                        Cfg_WriteSave(dlg, ofn.lpstrFile);
                    }
                } else {
                    if (GetOpenFileNameCompat(&ofn)) {
                        Cfg_ReadSave(dlg, ofn.lpstrFile);
                    }
                }
            }break;
            case MNU_CLEAR:Dlg_ClearAll(dlg);break;
            case MNU_PWAD:Dlg_ClearPWAD(dlg);break;
            /* Open and close the multiplay panel */
              case BTN_PANEL:
                  rct.top=0;rct.left=0;rct.right=270;rct.bottom=(cfg.dlgmode)?(282):(210);
                  cfg.dlgmode=(cfg.dlgmode)?(0):(1);
                  AdjustWindowRect(&rct,WS_POPUP|WS_CAPTION,0);
                  SetWindowPos(dlg,0,0,0,rct.right-rct.left,rct.bottom-rct.top,SWP_NOMOVE|SWP_NOZORDER);
                  SendDlgItemMessage(dlg,BTN_PANEL,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE((cfg.dlgmode)?(ICO_DOWN):(ICO_UP))));
              break;
            case BTN_ADD: {
                TCHAR tmpfn[MAX_PATH];
                OPENFILENAME ofn;
                if(pwad[MAX_PWAD-1]) { MessageBox(dlg,TEXT("Too many PWADs loaded!"),TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION); break; }
                memset(tmpfn, 0, sizeof(tmpfn));
                memset(&ofn, 0, sizeof(ofn));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = dlg;
                ofn.hInstance = GetModuleHandle(NULL);
                ofn.lpstrFilter=TEXT("ZDoom Addon Files (*.wad,*.deh,*.bex,*.zip,*.pk3)\0*.wad;*.deh;*.bex;*.zip;*.pk3\0All Files (*.*)\0*.*\0");
                ofn.Flags=OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_ALLOWMULTISELECT;
                ofn.lpstrTitle=TEXT("Load an External File");
                ofn.lpstrDefExt=TEXT("wad");
                /* Get the file and set the editbox */
                ofn.lpstrFile = tmpfn;
                ofn.nMaxFile  = MAX_PATH;
                if(!GetOpenFileNameCompat(&ofn)){break;}
                i=SendDlgItemMessage(dlg,LST_PWAD,LB_GETCOUNT,0,0);
                if(!ofn.nFileExtension){ /* Multiple files */
                    TCHAR *tmp, tmp2[MAX_PATH*2];
                    memset(tmp2 ,0 , sizeof(tmp2));
                    tmp=ofn.lpstrFile+ofn.nFileOffset;
                    for( ;i<MAX_PWAD&&tmp[0];i++){
                        wsprintf(tmp2, TEXT("%s\\%s"),ofn.lpstrFile,tmp);
                        if(!Dlg_AddPWAD(dlg,tmp2)){i--;}
                        tmp+=lstrlen(tmp)+1;
                        if(i+1>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION);break;}
                    }
                }else{Dlg_AddPWAD(dlg,ofn.lpstrFile);}
            }break;
            case BTN_REM: OnDeletePWAD(dlg); break;
            case BTN_UP: { /* Move up */
                HWND ithwnd = GetDlgItem(dlg,LST_PWAD);
                if((i=SendMessage(ithwnd,LB_GETCURSEL,0,0))!=LB_ERR&&i!=0){
                    TCHAR *tmp;
                    /* Move the entry in the window */
                    SendMessage(ithwnd,LB_INSERTSTRING,i-1,(LPARAM)GetFNinPath(pwad[i]));
                    SendMessage(ithwnd,LB_DELETESTRING,i+1,0);
                    SendMessage(ithwnd,LB_SETCURSEL,i-1,0);
                    /* Move the item */
                    tmp=pwad[i-1]; pwad[i-1]=pwad[i]; pwad[i]=tmp;
                }
            }break;
            case BTN_DWN: { /* Move down */
                HWND ithwnd = GetDlgItem(dlg,LST_PWAD);
                if((i=SendMessage(ithwnd, LB_GETCURSEL,0,0))!=LB_ERR&&i!=SendMessage(ithwnd, LB_GETCOUNT,0,0)-1){
                    TCHAR *tmp;
                    /* Move the entry in the window */
                    SendMessage(ithwnd, LB_INSERTSTRING,i+2,(LPARAM)GetFNinPath(pwad[i]));
                    SendMessage(ithwnd, LB_DELETESTRING,i,0);
                    SendMessage(ithwnd, LB_SETCURSEL,i+1,0);
                    /* Move the item */
                    tmp=pwad[i+1]; pwad[i+1]=pwad[i]; pwad[i]=tmp;
                }
            }break;
            /* Menu Selections */
            case MNU_OPTIONS: {
                HWND ithwnd;
                DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_OPTIONS),dlg,ConfigProc);
                /* Fill the port and IWAD arrays */
                ithwnd = GetDlgItem(dlg, LST_PORT);
                SendMessage(ithwnd, CB_RESETCONTENT, 0, 0);
                for (i=0; i < MAX_ITEM && port[i]; i++){
                    SendMessageFNb(ithwnd,CB_ADDSTRING,0, port[i]->name, port[i]->avail);
                }
                SendMessage(ithwnd, CB_SETCURSEL, 0, 0);

                ithwnd = GetDlgItem(dlg, LST_IWAD);
                SendMessage(ithwnd,LB_RESETCONTENT,0,0);
                for (i=0; i<MAX_ITEM && iwad[i]; i++) {
                    SendMessageFNb(ithwnd,LB_ADDSTRING,0, iwad[i]->name, iwad[i]->avail);
                }
                SendMessage(ithwnd, LB_SETCURSEL, 0, 0);

                SendDlgItemMessage(dlg, LST_WARP, CB_RESETCONTENT, 0, 0);
                if (SendMessage(ithwnd, LB_GETCURSEL, 0, 0) != LB_ERR) {
                    Dlg_PopulateWarp(dlg, iwad[Cfg_GetSel(0,iwad)]->path);
                }
            }break;
            case MNU_ABOUT:
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_ABOUT), dlg, AboutProc);break;
            /* Ways to exit */
            case IDCANCEL:
                SendMessage(dlg, WM_CLOSE,0,0); exit(0); break;
            case IDOK:
                Dlg_Launch(dlg, 0); break;
        }break;
    }break;

    case WM_DROPFILES: { /* Add files dropped onto the pwad box */
        TCHAR tmp2[MAX_PATH];
        r=DragQueryFile((HDROP)wp,0xFFFFFFFF,0,0);
        for(i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0);q<r;i++) {
            if(i>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,TEXT("Error!"),MB_OK|MB_ICONEXCLAMATION);break;}
            DragQueryFile((HDROP)wp,q,tmp2,MAX_PATH);
            if(!Dlg_AddPWAD(dlg,tmp2)) i--;
            q++;
        }
        DragFinish((HDROP)wp);
    }break;
    case WM_CLOSE:Dlg_Quit(dlg,1); exit(0);break;
    }

    return 0;
}

/*
#ifdef _UNICODE
#define __tgetmainargs __wgetmainargs
#else
#define __tgetmainargs __getmainargs
#endif
int WINAPI WinMain(HINSTANCE inst,HINSTANCE pinst,LPSTR cline,int cshow)
int MyMain(void)
*/
TCHAR g_pgmptr[MAX_PATH];

/* Avoid linking with msvcrt */
size_t __cdecl strlen(const char *str) { return lstrlenA(str); }
size_t __cdecl wcslen(const wchar_t *str) { return lstrlenW(str); }

/* Does not return NULL if it was unable to find the char! */
TCHAR *lstrchr(const TCHAR *str, const TCHAR c)
{
    while(*str != c) {
        if(!*str) return NULL;
        str++;
    }
    return (TCHAR *)str;
}

/* Does not return NULL if it was unable to find the char! */
TCHAR *lstrrchr(const TCHAR *str, const TCHAR c)
{
    const TCHAR *ostr=str;
    while(*str++);
    while(ostr < str && *str != c) {
        str--;
    }
    return (TCHAR *)str;
}

TCHAR *GetFNinPath(const TCHAR *p)
{
    int i=0;

    while(p[++i] != '\0');
    while(i >= 0 && p[i] != TEXT('\\') && p[i] != TEXT('/')) i--;
    i++;
    i += (p[i] == TEXT('\\') || p[i] == TEXT('/'));

    return (TCHAR*)&p[i]; /* first char of the filename */
}

static const TCHAR *ParamsFromCmdline(const TCHAR *cmdl)
{
    /* (")(PATH\TO\)PROG().exe(")    COMMAND */
    /* We want to cmdl to point      ^ HERE !*/
    /* in case it starts with " we need to go to the next " */
    if (cmdl[0] == TEXT('"')) {
        do {
            cmdl++;
        } while(*cmdl && *cmdl != TEXT('"'));
    } else {
        while(*cmdl && *cmdl != TEXT(' ') && *cmdl != TEXT('\t')) {
            cmdl++;
        }
    }
    cmdl += !!*cmdl; /* Skip the " or the ' ' */

    /* Skip eventual remaining spaces/tabs. */
    while(*cmdl == TEXT(' ') || *cmdl == TEXT('\t')) cmdl++;

    return cmdl;
}

int MyMain(void)
{
    HINSTANCE inst;
    TCHAR *cmdl;

    inst = GetModuleHandle(NULL);

    /* Set up the INI string */
    GetModuleFileName(NULL, g_pgmptr, MAX_PATH);
    lstrcpy_s(cfg.ini, countof(cfg.ini), g_pgmptr);
    lstrrchr(cfg.ini, TEXT('\\'))[1] = '\0';
    lstrcat_s(cfg.ini, countof(cfg.ini), TEXT("zdl.ini"));

    /* We need the command line for the INI thingy later */
    cmdline = NULL;
    cmdl = GetCommandLine();
    if (cmdl) {
        cmdline = (TCHAR*)ParamsFromCmdline(cmdl);
    }

    Cfg_LoadConfig(); /* Load the configuration file */

    /* GoooOOooooOOOOOoooOOO! */
    return DialogBox(inst, MAKEINTRESOURCE(DLG_MAIN), HWND_DESKTOP, MainProc);
}
