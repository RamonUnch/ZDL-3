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
* zdl.h : Prototypes and defines for stuff
**********************************************************************/
#include <windows.h>
#include <stdio.h>
#include <direct.h>

#define countof(x) ( sizeof(x)/sizeof(x[0]) )
 //////////////////////////
// Max values
#define MAX_ITEM 32
#define MAX_PWAD 64
#define MAX_NAME 48

 //////////////////////////
// Strings
#define STR_MAXPWAD TEXT("Too many files loaded! Not all files were added to the list.")
#define STR_NOITEMS TEXT("You must have at least 1 port and IWAD defined and available\n\nSelect ZDL -> Options to configure ports and IWADS")
#define STR_NEEDPRT TEXT("This save specifies the port \"%s\" which is unavailable.\nDo you want to use \"%s\" instead?")
#define STR_NEEDIWD TEXT("This save specifies the IWAD \"%s\" which is unavailable.\nDo you want to use \"%s\" instead?")

 //////////////////////////
// Data structs
typedef struct{ // Options
	char dlgmode;
	char launch;
	char autoclose;
	char padd___;
	TCHAR always[128];
	// Internal
	TCHAR ini[MAX_PATH];
	char assoc[8];
}CFG; extern CFG cfg;
typedef struct{ // IWAD defs
	char avail;
	TCHAR name[MAX_NAME];
	TCHAR path[MAX_PATH];
}ITEM;
extern ITEM *iwad[MAX_ITEM+1],*port[MAX_ITEM+1];
extern TCHAR *pwad[MAX_PWAD+1];
extern short arg1,arg2,arg3; // Needed for the config dialogs
extern TCHAR g_pgmptr[MAX_PATH];

 //////////////////////////
// zdl.c
TCHAR *lstrchr(const TCHAR *str, const TCHAR c);
TCHAR *lstrrchr(const TCHAR *str, const TCHAR c);
TCHAR *GetFNinPath(const TCHAR *p);

 //////////////////////////
// sub.c
INT_PTR CALLBACK AboutProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp);
INT_PTR CALLBACK AssocProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp);
INT_PTR CALLBACK FileProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp);

 //////////////////////////
// cfg.c
void RegisterFileType(TCHAR *ext,TCHAR *type,TCHAR *nicetype,TCHAR *exe,TCHAR* command,int icon);
void Cfg_LoadConfig();
int Cfg_ReadSave(HWND dlg, const TCHAR *file);
void Cfg_WriteSave(HWND dlg, const TCHAR *fptr);
int Cfg_GetSel(int sel,ITEM **item);
DWORD mySHDeleteKey(HKEY hkey, LPCTSTR pszSubKey);
#define SHDeleteKey mySHDeleteKey

 //////////////////////////
// dlg.c
void Dlg_Launch(HWND dlg,char prompt);
int Dlg_AddPWAD(HWND dlg, const TCHAR *file);
void Dlg_ClearPWAD(HWND dlg);
void Dlg_ClearAll(HWND dlg);
void Dlg_Quit(HWND dlg,char save);
void Dlg_PopulateWarp(HWND dlg, TCHAR *file);

 //////////////////////////
// zdl.rc
#define IDC_STATIC -1
// Icons
#define ICO_ICON 200
#define ICO_SAVE 201
#define ICO_WAD 202
#define ICO_PATCH 203
#define ICO_UP 204
#define ICO_DOWN 205
#define BMP_LOGO 206
// Main Dialog
#define DLG_MAIN 100
	// External files
	#define LST_PWAD 1000
	#define BTN_ADD 1001
	#define BTN_REM 1002
	#define BTN_UP 1003
	#define BTN_DWN 1004
	// System
	#define LST_PORT 1005
	#define LST_IWAD 1006
	#define LST_WARP 1007
	#define LST_SKILL 1008
	#define EDT_EXTRA 1009
	// Buttons
	//#define BTN_EXIT 1010
	#define BTN_ZDL 1011
	#define BTN_PANEL 1012
	#define BTN_LAUNCH 1013
	// Multiplay panel
	#define LST_GAME 1014
	#define EDT_HOST 1015
	#define LST_PLAYERS 1016
	#define EDT_FRAGS 1017
	#define EDT_DMF 1018
	#define EDT_DMF2 1019
// Options Dialog
#define DLG_OPTIONS 101
	//#define EDT_EXTRA // used in DLG_MAIN
	#define CHK_LAUNCH 1020
	#define CHK_AUTOCLOSE 1032
	//#define BTN_ZDL // used in DLG_MAIN
	//#define LST_PORT // used in DLG_MAIN
	//#define BTN_UP // used in DLG_MAIN
	//#define BTN_DWN // used in DLG_MAIN
	//#define BTN_ADD // used in DLG_MAIN
	//#define BTN_REM // used in DLG_MAIN
	//#define LST_IWAD // used in DLG_MAIN
	#define BTN_IUP 1021
	#define BTN_IDWN 1022
	#define BTN_IADD 1023
	#define BTN_IREM 1024
	//#define BTN_OK 1025
// File Dialog
#define DLG_FILE 102
	#define EDT_NAME 1026
	#define EDT_PATH 1027
	#define BTN_BROWSE 1028
	//#define BTN_EXIT // used in DLG_MAIN
	//#define BTN_OK // used in DLG_OPTIONS
// Association Dialog
#define DLG_ASSOC 103
	#define CHK_ZDL 1029
	#define CHK_WAD 1030
	#define CHK_DEH 1031
	#define CHK_PK3 1032
	#define CHK_ZIP 1033
	//#define BTN_OK // used in DLG_OPTIONS
// About Dialog
#define DLG_ABOUT 104
	//#define BTN_OK // used in DLG_OPTIONS
// Menu
#define MNU_MENU 500
	// Actions
		#define MNU_CMD 2000
		#define MNU_PWAD 2001
		#define MNU_CLEAR 2002
	#define MNU_LOAD 2003
	#define MNU_SAVE 2004
	#define MNU_OPTIONS 2005
	#define MNU_ABOUT 2006


 //////////////////////////////////////////
//
#define calloc(x,y) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (x)*(y) )
#define free(x) HeapFree( GetProcessHeap(), 0, (x) )
#define exit(x) ExitProcess( x )
#define FileExists(fn) (GetFileAttributes(fn) != 0xFFFFFFFF)

 //////////////////////////////////////////
//
void lstrcpy_sA(char *__restrict__ d, size_t N, const char *src);
void lstrcpy_sW(wchar_t *__restrict__ d, size_t N, const wchar_t *src);
char *lstrcat_sA(char *__restrict__ d, const size_t N, const char *__restrict__ s);
wchar_t *lstrcat_sW(wchar_t *__restrict__ d, const size_t N, const wchar_t *__restrict__ s);
#ifdef UNICODE
# define lstrcpy_s lstrcpy_sW
# define lstrcat_s lstrcat_sW
#else
# define lstrcpy_s lstrcpy_sA
# define lstrcat_s lstrcat_sA
#endif
