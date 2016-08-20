#include <stdio.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "w5inarow.h"
#include "5inarow2.h"
#include "menu.h"
#include "version.h"
#include "macro.h"

short int DlgInt1, DlgInt2, DlgTurnP, DlgTurnC;
long int DlgLong;
char *lpszTitle;
HBRUSH hbrush, hbrushturn;
int true = TRUE, false = FALSE;
static int lastx, lasty;
extern int size;
extern FLAGS flag;
extern long int opp_remaining_time, com_remaining_time;
extern HANDLE hStats, hPallete, hClock;
extern char prompt_title[];
extern int ShowMessageFromWin(char *, HWND);

void ConvertTimeToString(char *time_str, long int time_num);


BOOL CALLBACK GetNumDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	BOOL ok;
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
      case WM_INITDIALOG:
         SetWindowText(hDlg,lpszTitle);
			SetDlgItemText (hDlg, IDC_PROMPT, prompt_title);
			SetDlgItemInt ( hDlg, IDC_NUM, DlgInt1, TRUE);
			return TRUE;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
			{
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
		return true;
#endif

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDOK:
				DlgInt1 = GetDlgItemInt (hDlg, IDC_NUM, &ok, true);
				if (ok) EndDialog ( hDlg, true);
                	return true;
			case IDCANCEL:
				EndDialog ( hDlg, FALSE);
				return 0;
         }
    }
	return FALSE;
}

BOOL CALLBACK TimeDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	char num_str[20];
	float search_secs;
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
      case WM_INITDIALOG:
		 sprintf(num_str,"%.2f",(float)DlgLong/1000.0);
         SetWindowText(hDlg,lpszTitle);
		 SetDlgItemText(hDlg, IDC_MRTEXT, prompt_title);
		 SetDlgItemText(hDlg, IDC_MAXRT, num_str);
		 return TRUE;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
			{
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
		return true;
#endif

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_OK:
				if (GetDlgItemText (hDlg, IDC_MAXRT, num_str, 6)) {
					search_secs = atof(num_str);
					DlgLong = 1000.0*search_secs;
					EndDialog ( hDlg, true);
				}
                return true;
			case IDC_CANCEL:
				EndDialog ( hDlg, FALSE);
				return 0;
         }
    }
	return FALSE;
}

BOOL CALLBACK StatDlgProc ( HWND hDlg, UINT Message,
                               WPARAM wParam, LPARAM lParam)
{
	hbrush = CreateSolidBrush(RGB(192,192,192));

   switch (Message)
    {
      case WM_INITDIALOG:
         return (true);

#ifdef WIN32
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
         {
          case CTLCOLOR_BTN:
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
      return true;
#endif

      case WM_COMMAND:
         if (LOWORD(wParam)==IDOK)
          {
destroy:
            DestroyWindow(hDlg);
				flag.post=false;
            return true;
          }
         break;

      case WM_SYSCOMMAND:
         if ((wParam & 0xFFF0) == SC_CLOSE) goto destroy;
         break;
    }

    return (false);              /* Didn't process a message    */
}

BOOL CALLBACK ClockDlgProc ( HWND hDlg, UINT Message,
                               WPARAM wParam, LPARAM lParam)
{
	char time_str[20];
	hbrush = CreateSolidBrush(RGB(192,192,192));

   switch (Message)
    {
      case WM_INITDIALOG:
		 ConvertTimeToString(time_str,opp_remaining_time);
		 SetDlgItemText(hDlg,IDC_OPPTIME,time_str);
		 ConvertTimeToString(time_str,com_remaining_time);
		 SetDlgItemText(hDlg,IDC_COMTIME,time_str);
         return (true);

#ifdef WIN32
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
         {
          case CTLCOLOR_BTN:
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
      return true;
#endif

      case WM_SYSCOMMAND:
         if ((wParam & 0xFFF0) == SC_CLOSE) {
		 	DestroyWindow(hDlg);
            return true;
		 }
         break;
    }

    return (false);              /* Didn't process a message    */
}

BOOL CALLBACK AboutDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
      case WM_INITDIALOG:
         return true;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
		  return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
         {
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
      return true;
#endif

      case WM_COMMAND:
         if (LOWORD(wParam)==IDOK)
           {
             EndDialog ( hDlg, true);
             return true;
           }
    }
   return false;
}

BOOL CALLBACK VersionDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	char str[64];
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
      case WM_INITDIALOG:
		 sprintf(str,"This is Windows 5-in-a-row version %s (%s).",Version,Date);
		 SetDlgItemText(hDlg,IDC_VERSION,str);
         return true;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
		  return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
         {
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
      return true;
#endif

      case WM_COMMAND:
         if (LOWORD(wParam)==IDOK)
           {
             EndDialog ( hDlg, true);
             return true;
           }
    }
   return false;
}

BOOL CALLBACK GetTurnDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	hbrushturn = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
		case WM_INITDIALOG:
			CheckRadioButton(hDlg, IDC_HTURN, IDC_CTURN, IDC_HTURN);
			return true;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
		  return (int)hbrushturn;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
			{
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrushturn;
         }
		return true;
#endif

      case WM_COMMAND:
         switch (LOWORD(wParam))
			 {
				case IDC_OK:

					DlgTurnP = IsDlgButtonChecked (hDlg, IDC_HTURN);
					DlgTurnC = IsDlgButtonChecked (hDlg, IDC_CTURN);
					flag.edit = 0;
					EndDialog ( hDlg, true);
					return true;

				case IDC_CANCEL:
					flag.edit = 1;
					EndDialog ( hDlg, FALSE);
               		break;
				default:
					break;
         }
    }
	return FALSE;
}

BOOL CALLBACK PropsDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	BOOL ok;
	static int StartDlgInt1, StartDlgInt2;
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
	 {
		case WM_INITDIALOG:
			StartDlgInt1 = DlgInt1;
			StartDlgInt2 = DlgInt2;
			SetWindowText(hDlg,lpszTitle);
			/*EnableScrollBar(GetDlgItem(hDlg, IDC_BSIZESCROLL), SB_CTL, ESB_ENABLE_BOTH);*/
			SetScrollRange(GetDlgItem(hDlg, IDC_BSIZESCROLL), SB_CTL, 3, 20, TRUE);
			SetScrollPos(GetDlgItem(hDlg, IDC_BSIZESCROLL), SB_CTL, DlgInt1, TRUE);
			SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
			SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
			if (flag.standard_gomoku)
				CheckDlgButton(hDlg, IDC_STANDARD, BST_CHECKED);
			else
				CheckDlgButton(hDlg, IDC_STANDARD, BST_UNCHECKED);
			return TRUE;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
		case WM_CTLCOLOR:
        switch(HIWORD(lParam))
			{
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
		return true;
#endif
		case WM_VSCROLL:
#ifdef WIN32
			if (lParam == (long)GetDlgItem(hDlg, IDC_BSIZESCROLL)) {
				if (LOWORD(wParam) == SB_LINEDOWN && DlgInt1 > 3) {
					DlgInt1--;
					SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
				}
				if (LOWORD(wParam) == SB_LINEUP && DlgInt1 < 19) {
					DlgInt1++;
					SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
				}
			}
			if (lParam == (long)GetDlgItem(hDlg, IDC_CONSCROLL)) {
				if (LOWORD(wParam) == SB_LINEDOWN && DlgInt2 > 1) {
					DlgInt2--;
					SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
				}
				if (LOWORD(wParam) == SB_LINEUP && DlgInt2 < 10) {
					DlgInt2++;
					SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
				}
			}
#else
			if (HIWORD(lParam) == (unsigned short)GetDlgItem(hDlg, IDC_BSIZESCROLL)) {
				if (LOWORD(wParam) == SB_LINEDOWN && DlgInt1 > 3) {
					DlgInt1--;
					SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
				}
				if (LOWORD(wParam) == SB_LINEUP && DlgInt1 < 19) {
					DlgInt1++;
					SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
				}
			}
			if (HIWORD(lParam) == (unsigned short)GetDlgItem(hDlg, IDC_CONSCROLL)) {
				if (LOWORD(wParam) == SB_LINEDOWN && DlgInt2 > 1) {
					DlgInt2--;
					SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
				}
				if (LOWORD(wParam) == SB_LINEUP && DlgInt2 < 10) {
					DlgInt2++;
					SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
				}
			}
#endif
			if (DlgInt1 != 19 || DlgInt2 != 5)
				CheckDlgButton(hDlg,IDC_STANDARD,BST_UNCHECKED);
			return (true);

		case WM_COMMAND:
			switch (LOWORD(wParam))
          {
			  	case IDC_STANDARD:
					if (IsDlgButtonChecked(hDlg,IDC_STANDARD)) {
					DlgInt1 = 19;
					SetDlgItemInt(hDlg, IDC_BSIZE, DlgInt1, FALSE);
					DlgInt2 = 5;
					SetDlgItemInt(hDlg, IDC_CONNECTS, DlgInt2, FALSE);
					}
					return true;
				case IDOK:
					DlgInt1 = GetDlgItemInt (hDlg, IDC_BSIZE, &ok, true);
					DlgInt2 = GetDlgItemInt (hDlg, IDC_CONNECTS, &ok, true);
					if (DlgInt1 > 19 || DlgInt1 < 3) {
						char str[50];
						sprintf(str,"Board size must from 3 to 19.");
						ShowMessageFromWin(str, hDlg);
						return (false);
					}
					if (DlgInt2 > 10 || DlgInt2 < 1) {
						char str[50];
						sprintf(str,"Connections to win must be from 1 to 10.");
						ShowMessageFromWin(str, hDlg);
						return (false);
					}
					flag.standard_gomoku = IsDlgButtonChecked(hDlg,IDC_STANDARD);
					if (ok) EndDialog ( hDlg, true);
					return true;

				case IDCANCEL:
					DlgInt1 = StartDlgInt1;
					DlgInt2 = StartDlgInt2;
					EndDialog ( hDlg, FALSE);
					return true;
			}

	 }
	return FALSE;
}

BOOL CALLBACK PalleteDlgProc ( HWND hDlgPal, UINT Message,
                               WPARAM wParam, LPARAM lParam)
{
	hbrush = CreateSolidBrush(RGB(192,192,192));
   switch (Message)
    {
      case WM_INITDIALOG:
         return (true);

#ifdef WIN32
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
        return (int)hbrush;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
         {
          case CTLCOLOR_BTN:
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrush;
         }
      return true;
#endif

	  case WM_COMMAND:
		if (flag.edit) break;
		if ((flag.both || flag.compmove) && LOWORD(wParam) != IDM_QUIT && LOWORD(wParam)
			!= IDM_BOTH && LOWORD(wParam) != IDM_FORCE)
			return true;
		PostMessage(GetParent(hDlgPal), Message, LOWORD(wParam), lParam);
        return true;

      case WM_SYSCOMMAND:
         if ((wParam & 0xFFF0) == SC_CLOSE) {
			 DestroyWindow(hDlgPal);
			 return true;
		 }
         break;
    }

    return (false);              /* Didn't process a message    */
}

BOOL CALLBACK ColorDlgProc (HWND hDlg, UINT Message,
											 WPARAM wParam, LPARAM lParam)
{
	hbrushturn = CreateSolidBrush(RGB(192,192,192));
   switch ( Message )
    {
		case WM_INITDIALOG:
			/* Init */
			return true;

#ifdef WIN32
      case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam,TRANSPARENT);
      case WM_CTLCOLORDLG:
		  return (int)hbrushturn;
#else
      case WM_CTLCOLOR:
        switch(HIWORD(lParam))
			{
          case CTLCOLOR_STATIC:
           SetBkMode((HDC)wParam,TRANSPARENT);
          case CTLCOLOR_DLG:
			  return hbrushturn;
         }
		return true;
#endif

      case WM_COMMAND:
         switch (LOWORD(wParam))
			 {
				case IDC_OK:

					DlgTurnP = IsDlgButtonChecked (hDlg, IDC_HTURN);
					DlgTurnC = IsDlgButtonChecked (hDlg, IDC_CTURN);
					flag.edit = 0;
					EndDialog ( hDlg, true);
					return true;

				case IDC_CANCEL:
					flag.edit = 1;
					EndDialog ( hDlg, FALSE);
               		break;
				default:
					break;
         }
    }
	return FALSE;
}

void
ShowScore (short int score)
{
  char msg[20];
  sprintf (msg, "%d", (int)score);
  SetDlgItemText(hStats,IDC_SCORE,msg);
}

void
ShowNodeCnt (long int NodeCnt, float nps)
{
	char msg[20];
	sprintf (msg,"%ld", NodeCnt);
	SetDlgItemText(hStats,IDC_NODE,msg);
	if (nps != -1) {
		sprintf(msg,"%ld", (long)nps);
		SetDlgItemText(hStats,IDC_NODESEC,msg);
	}
}

void ShowQuiesceNodeCnt(long int NodeCnt)
{
	char msg[20];
	sprintf(msg,"%ld",NodeCnt);
	SetDlgItemText(hStats,IDC_QUIESCENODES,msg);
}
void ShowSearchedMove(int move, int x, int y, int nbr_of_moves, int depth)
{
	char msg[20];
	if (nbr_of_moves != -1) {
		sprintf(msg,"%c %d %s(%d/%d) %sd:%d",'A'+x,size-y,(size-y > 9 ? "" : "  "),move ,nbr_of_moves,(move > 9 ? "" : " "),depth);
	}
	else
		sprintf(msg,"%c %d (%d)",'A'+x,size-y,move);
	SetDlgItemText(hStats,IDC_MOVE,msg);
}

void ShowBestSearchedMove(int move, int x, int y)
{
	char msg[20];
	if (x != -1 || y != -1) {
		sprintf(msg,"%c %d",'A'+x,size-y);
		lastx = x;
		lasty = y;
	}
	else
		sprintf(msg,"%c %d (%d)",'A'+lastx,size-lasty);
	SetDlgItemText(hStats,IDC_BESTMOVE,msg);
}

void ShowPV(int depth, int level, PVMOVE best_line[])
{
	int m;
	char str[30], pvstr[100];
	pvstr[0] = '\0';
	for (m=1;m<=depth+MAX_QUIESCE_DEPTH;m++) {
		if (best_line[m].move == -1 || depth == level) break;
		sprintf(str,"%c%d ",'A'+MOVEX(best_line[m].move),size-MOVEY(best_line[m].move));
		strcat(pvstr,str);
	}
	SetDlgItemText(hStats,IDC_BESTMOVE,pvstr);
}

void ConvertTimeToString(char *time_str, long int time_num)
			/* Converts miliseconds to the usual time format: HH:MM:SS */
{
	int hours, minutes, seconds;
	char str[20];

	hours = time_num / 3600000;
	minutes = (time_num % 3600000) / 60000;
	seconds = ((time_num % 3600000) % 60000) / 1000;

	sprintf(str,"%d:%s%d:%s%d",hours,(minutes<10?"0":""),minutes,(seconds<10?"0":""),seconds);
	strcpy(time_str,str);
}

void SetClockTime(long int time_num, int player)
{
	char time_str[20];

	ConvertTimeToString(time_str,time_num);

	SetDlgItemText(hClock,(player==OPP?IDC_OPPTIME:IDC_COMTIME),time_str);
}
