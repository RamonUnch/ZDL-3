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
* zdl.c : Window Procedures for minor dialogs
**********************************************************************/
#include "zdl.h"

INT_PTR CALLBACK AboutProc(HWND Dlg,UINT Msg,WPARAM wp,LPARAM lp){if(Msg==WM_COMMAND&&LOWORD(wp)==IDOK){EndDialog(Dlg,0);}return 0;}

INT_PTR CALLBACK AssocProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp){
    long sz=32;
    TCHAR tmp[64]={0};

    switch (msg) {
    case WM_INITDIALOG:
        // Check all already associated stuff.
        if(RegQueryValue(HKEY_CLASSES_ROOT, TEXT("ZDL.SaveFile"), tmp, &sz)==ERROR_SUCCESS && tmp[0])
            {SendDlgItemMessage(dlg, CHK_ZDL, BM_SETCHECK,1,0);cfg.assoc[0]=1; } sz=MAX_PATH;
        if(RegQueryValue(HKEY_CLASSES_ROOT, TEXT("ZDL.WADFile"),  tmp, &sz)==ERROR_SUCCESS && tmp[0])
            {SendDlgItemMessage(dlg, CHK_WAD, BM_SETCHECK,1,0);cfg.assoc[1]=1;}sz=MAX_PATH;
        if(RegQueryValue(HKEY_CLASSES_ROOT, TEXT("ZDL.PK3File"),  tmp, &sz)==ERROR_SUCCESS && tmp[0])
            {SendDlgItemMessage(dlg, CHK_PK3, BM_SETCHECK,1,0);cfg.assoc[3]=1;}sz=MAX_PATH;
        if(RegQueryValue(HKEY_CLASSES_ROOT, TEXT("ZDL.ZipFile"),  tmp, &sz)==ERROR_SUCCESS && tmp[0])
            {SendDlgItemMessage(dlg, CHK_ZIP, BM_SETCHECK,1,0);cfg.assoc[4]=1;}sz=MAX_PATH;
        if(RegQueryValue(HKEY_CLASSES_ROOT, TEXT("ZDL.PatchFile"),tmp, &sz)==ERROR_SUCCESS && tmp[0])
            {SendDlgItemMessage(dlg, CHK_DEH, BM_SETCHECK,1,0);cfg.assoc[2]=1;}sz=MAX_PATH;
        break;
    case WM_COMMAND:
        switch(HIWORD(wp)) {
        case BN_CLICKED:
            if(LOWORD(wp)==IDOK){
                // Save Files
                if (SendDlgItemMessage(dlg,CHK_ZDL,BM_GETCHECK,0,0)) {
                    if (!cfg.assoc[0]) { // Set the value
                        RegisterFileType(TEXT(".zdl"),TEXT("ZDL.SaveFile"),TEXT("ZDL Saved Configuration File"),g_pgmptr,TEXT("\"%1\""),1);
                    } else { // Delete keys
                        RegDeleteKey(HKEY_CLASSES_ROOT,TEXT(".zdl"));
                        SHDeleteKey(HKEY_CLASSES_ROOT,TEXT("ZDL.SaveFile"));
                    }
                }
                if (SendDlgItemMessage(dlg,CHK_WAD,BM_GETCHECK,0,0)) {
                    if (!cfg.assoc[1]) { // Set the value
                        RegisterFileType(TEXT(".wad"),TEXT("ZDL.WADFile"),TEXT("Doom Engine Data File"), g_pgmptr,TEXT("\"%1\""), 2);
                    } else { // Delete keys
                        RegDeleteKey(HKEY_CLASSES_ROOT,TEXT(".wad"));
                        SHDeleteKey(HKEY_CLASSES_ROOT,TEXT("ZDL.WADFile"));
                    }
                }
                if (SendDlgItemMessage(dlg,CHK_DEH,BM_GETCHECK,0,0)) {
                    if (!cfg.assoc[2]) { // Set the value
                        RegisterFileType(TEXT(".deh"),TEXT("ZDL.PatchFile"),TEXT("DeHackEd Patch"),g_pgmptr,TEXT("\"%1\""),3);
                        RegisterFileType(TEXT(".bex"),TEXT("ZDL.PatchFile"),TEXT("DeHackEd Patch"),g_pgmptr,TEXT("\"%1\""),3);
                    } else { // Delete keys
                        RegDeleteKey(HKEY_CLASSES_ROOT, TEXT(".deh"));
                        RegDeleteKey(HKEY_CLASSES_ROOT, TEXT(".bex"));
                        SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("ZDL.PatchFile"));
                    }

                }
                if (SendDlgItemMessage(dlg, CHK_PK3,BM_GETCHECK,0,0)) {
                    if (!cfg.assoc[3]) { // Set the value
                        RegisterFileType(TEXT(".pk3"),TEXT("ZDL.PK3File"),TEXT("Doom Engine Data File"),g_pgmptr,TEXT("\"%1\""),2);
                    } else { // Delete keys
                        RegDeleteKey(HKEY_CLASSES_ROOT,TEXT(".pk3"));
                        SHDeleteKey(HKEY_CLASSES_ROOT,TEXT("ZDL.PK3File"));
                    }
                }
                if (SendDlgItemMessage(dlg,CHK_ZIP,BM_GETCHECK,0,0)) {
                    if (!cfg.assoc[4]) { // Set the value
                        RegisterFileType(TEXT(".zip"),TEXT("ZDL.ZipFile"),TEXT("Doom Engine Data File"),g_pgmptr,TEXT("\"%1\""),2);
                    } else { // Delete keys
                        RegDeleteKey(HKEY_CLASSES_ROOT,TEXT(".zip"));
                        SHDeleteKey(HKEY_CLASSES_ROOT,TEXT("ZDL.ZipFile"));
                    }
                }
                EndDialog(dlg, 0);
            } else if(LOWORD(wp)==IDCANCEL) {
                EndDialog(dlg, 0);
            }break;
    } break;
    }
    
    return 0;
}

INT_PTR CALLBACK FileProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp)
{
    ITEM **item=0;
    switch(msg){
    case WM_INITDIALOG: {
        item=(arg1==LST_PORT)?(port):(iwad);
        if(arg2!=-1){ // Only set the text in the editboxes if it's not for a new entry
            SendMessage(dlg,WM_SETTEXT,0,(arg1==LST_PORT)?((LPARAM)TEXT("Edit Port")):((LPARAM)TEXT("Edit IWAD")));
            SendDlgItemMessage(dlg,EDT_NAME,WM_SETTEXT,0,(LPARAM)item[arg2]->name);
            SendDlgItemMessage(dlg,EDT_PATH,WM_SETTEXT,0,(LPARAM)item[arg2]->path);
        }else{SendMessage(dlg,WM_SETTEXT,0,(arg1==LST_PORT)?((LPARAM)TEXT("New Port")):((LPARAM)TEXT("New IWAD")));}
        SendDlgItemMessage(dlg,EDT_NAME,EM_LIMITTEXT,MAX_NAME,0);
        SendDlgItemMessage(dlg,EDT_PATH,EM_LIMITTEXT,MAX_PATH,0);
    }break;
    case WM_COMMAND:
        switch(HIWORD(wp)) {
        case BN_CLICKED:
            switch(LOWORD(wp)) {
            case BTN_BROWSE: { // Set up the path
                // Set up the file dialog
                TCHAR tmpfn[MAX_PATH];
                OPENFILENAME ofn;
                memset(tmpfn, 0, sizeof(tmpfn));
                memset(&ofn, 0, sizeof(ofn));

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = dlg;
                ofn.hInstance = GetModuleHandle(NULL);
                ofn.lpstrFile   = tmpfn;
                ofn.nMaxFile  = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;
                ofn.lpstrFilter = arg1==LST_PORT? TEXT("Executable Files (*.exe)\0*.exe"): TEXT("IWAD Files (*.wad)\0*.wad");
                ofn.lpstrDefExt = arg1==LST_PORT? TEXT("exe"): TEXT("wad");
                ofn.lpstrTitle  = arg1==LST_PORT? TEXT("Browse for Port Executable"): TEXT("Browse for IWAD");
                // Get the file and set the editbox
                if (GetOpenFileName(&ofn)) {
                    SendDlgItemMessage(dlg, EDT_PATH, WM_SETTEXT, 0, (LPARAM)ofn.lpstrFile);
                }
            }break;
            case IDOK: // Save the changes
                item=(arg1==LST_PORT)?(port):(iwad);
                if(!SendDlgItemMessage(dlg,EDT_NAME,WM_GETTEXTLENGTH,0,0)){MessageBox(dlg,TEXT("You must type a name."),TEXT("Error"),MB_OK|MB_ICONINFORMATION);break;}
                if(!SendDlgItemMessage(dlg,EDT_PATH,WM_GETTEXTLENGTH,0,0)){MessageBox(dlg,TEXT("You must type a path."),TEXT("Error"),MB_OK|MB_ICONINFORMATION);break;}
                if(arg2==-1){ // If you are making a new item
                    item[arg3] = calloc(sizeof(ITEM), 1);
                } else {
                    arg3=arg2;
                }
                SendDlgItemMessage(dlg, EDT_NAME, WM_GETTEXT, MAX_NAME, (LPARAM)item[arg3]->name);
                SendDlgItemMessage(dlg, EDT_PATH, WM_GETTEXT, MAX_PATH, (LPARAM)item[arg3]->path);
                item[arg3]->avail = FileExists(item[arg3]->path);
                arg1=arg2=arg3=0; // Reset the temp vars
                /* Fall through */
            case IDCANCEL:
                EndDialog(dlg,0);break;
            }break;
        }break;
    }

    return 0;
}
