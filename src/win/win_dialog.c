/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Several dialogs for the application.
 *
 * Version:	@(#)win_dialog.c	1.0.11	2019/09/22
 *
 * Author:	Miran Grca, <mgrca8@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *		Copyright 2016-2019 Miran Grca.
 *		Copyright 2017-2019 Fred N. van Kempen.
 */
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <commdlg.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include "../86box.h"
#include "../device.h"
#include "../plat.h"
#include "../ui.h"
#include "win.h"


WCHAR	wopenfilestring[260];
char	openfilestring[260];
uint8_t	filterindex = 0;


int
ui_msgbox(int flags, void *arg)
{
    WCHAR temp[512];
    WCHAR *icon = NULL;
    TASKDIALOG_COMMON_BUTTON_FLAGS buttons = TDCBF_OK_BUTTON;
    WCHAR *str = NULL;
    WCHAR *instr = NULL;
    int res = 0;

    switch(flags & 0x1f) {
	case MBX_INFO:		/* just an informational message */
		icon = TD_INFORMATION_ICON;
		instr = plat_get_string(IDS_STRINGS);	    /* "86Box" */
		break;

	case MBX_ERROR:		/* error message */
		if (flags & MBX_FATAL) {
			icon = TD_ERROR_ICON;
			instr = plat_get_string(IDS_2050);    /* "Fatal Error"*/
		} else {
			icon = TD_WARNING_ICON;
			instr = plat_get_string(IDS_2049);    /* "Error" */
		}
		break;

	case MBX_QUESTION:	/* question */
		buttons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON;
		instr = plat_get_string(IDS_STRINGS);	    /* "86Box" */
		break;

	case MBX_QUESTION_YN:	/* question */
		buttons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
		instr = plat_get_string(IDS_STRINGS);	    /* "86Box" */
		break;
    }

    /* If ANSI string, convert it. */
    str = (WCHAR *)arg;
    if (flags & MBX_ANSI) {
	mbstowcs(temp, (char *)arg, strlen((char *)arg)+1);
	str = temp;
    } else {
	/*
	 * It's a Unicode string.
	 *
	 * Well, no, maybe not. It could also be one of the
	 * strings stored in the Resources. Those are wide,
	 * but referenced by a numeric ID.
	 *
	 * The good news is, that strings are usually stored
	 * in the executable near the end of the code/rodata
	 * segment. This means, that *real* string pointers
	 * usually have a pretty high (numeric) value, much
	 * higher than the numeric ID's.  So, we guesswork
	 * that if the value of 'arg' is low, its an ID..
	 */
	if (((uintptr_t)arg) < ((uintptr_t)65636))
		str = plat_get_string((intptr_t)arg);
    }

    /* At any rate, we do have a valid (wide) string now. */
    TaskDialog(hwndMain,	/* our main window */
	       NULL,		/* no icon resource */
	       plat_get_string(IDS_STRINGS), /* window caption */
	       instr,		/* main instruction */
	       str,		/* error message */
	       buttons,
	       icon,
	       &res);		/* pointer to result */

    /* Convert return values to generic ones. */
    if (res == IDNO) res = 1;
     else if (res == IDCANCEL) res = -1;
     else res = 0;

    return(res);
}


#if 0
int
msgbox_reset_yn(HWND hwnd)
{
    return(MessageBox(hwnd, plat_get_string(IDS_2051),
#endif


int
file_dlg_w(HWND hwnd, WCHAR *f, WCHAR *fn, int save)
{
    OPENFILENAME ofn;
    BOOL r;
    /* DWORD err; */

    /* Initialize OPENFILENAME */
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = wopenfilestring;

    /*
     * Set lpstrFile[0] to '\0' so that GetOpenFileName does
     * not use the contents of szFile to initialize itself.
     */
    memcpy(ofn.lpstrFile, fn, (wcslen(fn) << 1) + 2);
    ofn.nMaxFile = 259;
    ofn.lpstrFilter = f;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST;
    if (! save)
	ofn.Flags |= OFN_FILEMUSTEXIST;

    /* Display the Open dialog box. */
    if (save)
	r = GetSaveFileName(&ofn);
    else
	r = GetOpenFileName(&ofn);

    plat_chdir(usr_path);

    if (r) {
	wcstombs(openfilestring, wopenfilestring, sizeof(openfilestring));
	filterindex = ofn.nFilterIndex;

	return(0);
    }

    return(1);
}


int
file_dlg(HWND hwnd, WCHAR *f, char *fn, int save)
{
    WCHAR ufn[512];

    mbstowcs(ufn, fn, strlen(fn) + 1);

    return(file_dlg_w(hwnd, f, ufn, save));
}


int
file_dlg_mb(HWND hwnd, char *f, char *fn, int save)
{
    WCHAR uf[512], ufn[512];

    mbstowcs(uf, f, strlen(fn) + 1);
    mbstowcs(ufn, fn, strlen(fn) + 1);

    return(file_dlg_w(hwnd, uf, ufn, save));
}


int
file_dlg_w_st(HWND hwnd, int id, WCHAR *fn, int save)
{
    return(file_dlg_w(hwnd, plat_get_string(id), fn, save));
}


int
file_dlg_st(HWND hwnd, int id, char *fn, int save)
{
    return(file_dlg(hwnd, plat_get_string(id), fn, save));
}
