#include <windows.h>
#include <mmsystem.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <math.h>
#include <commdlg.h>
#include <time.h>

#include "w5inarow.h"
#include "5inarow2.h"
#include "menu.h"
#include "macro.h"
/*#ifndef __FLAT__
#error This Example must be compiled using 32 bit compiler
#endif*/


BOOL InitApplication(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT FAR PASCAL /*_export*/ MainWndProc(HWND hWnd, UINT message,
				WPARAM wParam, LPARAM lParam);
int ShowIntroMessages(HWND hWnd);
void drawLine(HDC hdc, int x, int y, int width, int length);
void drawBoard(HDC hdc);
int CheckMove(HDC hdc, int x, int y);
void PutTick(HDC hdc, int x, int y, int player);
void DrawTick(HDC hdc, int x, int y, int player);
void DrawPattern(HDC hdc, int x, int y);
void ShowResult(HWND hWnd, int winner, int win_on_time);
void ClearWindow(HWND hWnd);
void ShowMessageFromWin (char *, HWND);
void EditBoard(HWND hWnd);
void TurnOnMenuItems(HWND);
void TurnOffMenuItems(HWND);
static BOOL CreateSBar(HWND hwndParent,char *initialText,int nrOfParts);
void UpdateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags);
void InitializeStatusBar(HWND hwndParent,int nrOfParts);
void DestroyStatusBar(void);
void ProcessTimer(void);
DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName);
void closeMIDIDevice(void);
int getMIDIFileName(char *file_name);

BOOL CALLBACK GetNumDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TimeDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK StatDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK GetTurnDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PropsDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PalleteDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK VersionDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ClockDlgProc(HWND, UINT, WPARAM, LPARAM);

HWND mainwin, hWND, hWndStatusbar;
HANDLE hInst, hStats, hAccel, hPallete, hClock;
HDC hdc2;
POINT point;
BOOL CALLBACK (*lpfnDlgProc)(HWND, UINT, WPARAM, LPARAM);
extern void MakeMove(int,int);
extern MOVE CompMove(int,int);
extern int IsEmpty(int);
extern int LegalMove(int);
extern short DlgInt1, DlgInt2, DlgTurnC, DlgTurnP;
extern long int DlgLong;
extern char *lpszTitle;
static int player, lastplayer, depth=DEFAULT_DEPTH, depth2=DEFAULT_DEPTH;
static long last_opp_tick_count, last_com_tick_count, opp_start, opp_end;
long search_time, opp_remaining_time, com_remaining_time;
long game_time;
int move_num;
FLAGS flag;
enum {small,medium,large};
int total_mainwin_width = TOTAL_MAINWIN_WIDTH;
int total_mainwin_height = TOTAL_MAINWIN_HEIGHT;
int win_width[3] = { 266, 323, 418};
int win_type = DEFAULT_WIN_TYPE;
int size;
int connects;
int score_shift_connections;
int flash_delay;
extern int mainwin_width;
int tick_size;
int MainWin_XPos, MainWin_YPos;
BoardSq_T pressedsq[DEFAULT_SIZE*DEFAULT_SIZE];
char szMenu[] = "MENU_1";

char prompt_title[20];
char filter[]="5inarow files (*.fir)\0*.fir\0All files (*.*)\0*.*\0";
UINT wDeviceID;
static UINT idTimer;
HBITMAP hImage;

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		  LPSTR lpCmdLine, int nCmdShow)

{
	MSG msg;      // message
	MOVE move;
	HDC hdc;
	int winner;
	SYSTEMTIME time;
	long int start, end;
	char str[60], timestr[30], oldtimestr[30], midi_file[128];
	flag.quit = 0;
	flag.edit = 0;
	flag.post = 0;
	flag.havewinner = 0;
	flag.both = 0;
	flag.compmove = 0;
	flag.force = 0;
	flag.flash = 1;
	flag.flashing = 0;
	flag.standard_gomoku = 1;
	flag.sound = 1;
	flag.mididev = 0;
	flag.music = 1;
	flag.search_extensions = 0;
	flag.newticks = 1;
	flag.iterative_deepening = 1;
	flag.timed_game = 1;
	flag.ttable = 0;
	flag.savepv = 1;
	flag.opening_game = 0;
	size=DEFAULT_SIZE;
	connects=DEFAULT_CONNECTS;
	score_shift_connections = DEFAULT_SCORE_SHIFT_CONNECTIONS;
	flash_delay=DEFAULT_FLASH_DELAY;
	search_time = DEFAULT_SEARCH_TIME;
	game_time = DEFAULT_INITIAL_CLOCK_TIME;
	hInst=hInstance;
	hImage = LoadBitmap(hInstance,MAKEINTRESOURCE(IMAGE));
	InitGame();

	if (!InitApplication(hInstance))  // Initialize shared things
		return (FALSE); // Exits if unable to initialize

	/* Perform initializations that apply to a specific instance */
	if (((mainwin = InitInstance(hInstance, nCmdShow)) == NULL)) {
		EndGame();
		return (FALSE);
 	}
	hWND=mainwin;
	CreateSBar(hWND,"Ready",5);
	hAccel=LoadAccelerators(hInstance,szMenu);
	if (hAccel == NULL) {
		ShowMessage("Error: couldn't load accelerators");
		exit(1);
	}
	ShowIntroMessages(mainwin);
  	/*if (!InitButtons(hInstance, nCmdShow, mainwin))
	return (FALSE);*/
	DrawMenuBar(mainwin);
  	/* Acquire and dispatch messages until a WM_QUIT message is received. */

	hdc = GetDC(mainwin);
	MoveToEx(hdc,200,200,NULL);
	ReleaseDC(mainwin,hdc);
	if (flag.flash)
		CheckMenuItem(GetMenu(hWND),IDM_FLASH,MF_CHECKED);
	if (score_shift_connections == 2)
		CheckMenuItem(GetMenu(hWND),IDM_AGRESSIVEMODE,MF_CHECKED);
	if (flag.sound)
		CheckMenuItem(GetMenu(hWND),IDM_SOUND,MF_CHECKED);
	if (win_type == small) CheckMenuItem(GetMenu(hWND),IDM_SMALL,MF_CHECKED);
	else if (win_type == medium) CheckMenuItem(GetMenu(hWND),IDM_MEDIUM,MF_CHECKED);
	else CheckMenuItem(GetMenu(hWND),IDM_LARGE,MF_CHECKED);
	if (flag.music)
		CheckMenuItem(GetMenu(hWND),IDM_MUSIC,MF_CHECKED);
	if (getMIDIFileName(midi_file))
		playMIDIFile(hWND,midi_file);
	else {
		flag.music = 0;
		CheckMenuItem(GetMenu(hWND),IDM_MUSIC,MF_UNCHECKED);
	}
	CheckMenuItem(GetMenu(hWND),IDM_STYLE,(flag.newticks ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(hWND),IDM_ITERATIVEDEEPENING,(flag.iterative_deepening ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(hWND),IDM_TIMEDGAME,(flag.timed_game ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(hWND),IDM_TTABLE,(flag.ttable?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(GetMenu(hWND),IDM_SAVEPV, (flag.savepv ? MF_CHECKED : MF_UNCHECKED));
	while (1) {
		if (flag.havewinner || GetPieceNbr() == 0)
	 		EnableMenuItem(GetMenu(hWND),IDM_AGRESSIVEMODE,MF_ENABLED);
	 	else
			EnableMenuItem(GetMenu(hWND),IDM_AGRESSIVEMODE,MF_GRAYED);
		if (!flag.both && player == OPP) {
			TurnOnMenuItems(mainwin);
			UpdateStatusBar("Your turn",0,0);
			sprintf(str,"Size: %dx%d",size,size);
			UpdateStatusBar(str,1,0);
			sprintf(str,"%d in a row",connects);
			UpdateStatusBar(str,2,0);
			sprintf(str,"Depth: %d",depth);
			UpdateStatusBar(str,3,0);
			GetLocalTime(&time);
			strcpy(oldtimestr,timestr);
			if (time.wMinute < 10 && time.wMinute >= 0)
				sprintf(timestr,"Time: %d:0%d",(int)time.wHour,(int)time.wMinute);
			else
				sprintf(timestr,"Time: %d:%d",(int)time.wHour,(int)time.wMinute);
			if (strcmp(timestr,oldtimestr) != 0)
				UpdateStatusBar(timestr,4,0);
			if (!GetMessage(&msg, // message structure
		  	 0, // handle of window receiving the message
		  	 0, // lowest message to examine
		  	 0 // highest message to examine
		  	 )) break;
			if (!TranslateAccelerator (mainwin, hAccel, &msg)) {
				TranslateMessage(&msg); // Translates virtual key codes
				DispatchMessage(&msg);  // Dispatches message to window
			}
			if (flag.quit) {
				PostQuitMessage(0);
				player = OPP;
				flag.both = 0;
				continue;
			}
			if (player != COM) continue;
	 		else move_num++;
		}
	 	else if (flag.both && player == OPP) {
			TurnOffMenuItems(mainwin);
			UpdateStatusBar("X's turn",0,0);
			flag.compmove = TRUE;
			if (flag.timed_game)
				start = GetTickCount();
			move = CompMove(depth2, OPP);
			if (flag.timed_game) last_opp_tick_count = GetTickCount();
			if (flag.timed_game) {
				end = GetTickCount();
				opp_remaining_time -= (end-start);
				SetClockTime(opp_remaining_time,OPP);
			}
			hdc = GetDC(mainwin);
			PutTick(hdc, MOVEX(move.move)*tick_size+ST_COORDX, MOVEY(move.move)*tick_size+ST_COORDY, player);
			ReleaseDC(mainwin,hdc);
			MakeMove(move.move, OPP);
			move_num++;
			if (flag.sound) {
				if (!sndPlaySound("oppmove.wav",SND_ASYNC)) {
					sprintf(str,"Error playing sound file. Sound disabled");
					flag.sound = 0;
					ShowMessage(str);
				}



			}
			if (flag.quit) {
				PostQuitMessage(0);
				player = OPP;
				continue;
			}
			player = COM;
      		flag.compmove = FALSE;
			if (flag.both == TRUE)
				flag.force = FALSE;
	 	}
	 	else {
			if (flag.timed_game) last_com_tick_count = GetTickCount();
    		TurnOffMenuItems(mainwin);
			if (!flag.both) UpdateStatusBar("My turn. Thinking...",0,0);
			else UpdateStatusBar("O's turn",0,0);
			flag.compmove = TRUE;
			if (flag.timed_game)
				start = GetTickCount();
			move = CompMove(depth, COM);
			if (flag.timed_game) {
				end = GetTickCount();
				com_remaining_time -= (end-start);
				SetClockTime(com_remaining_time,COM);
				if (!flag.both) opp_start = GetTickCount();
			}

			if (flag.timed_game) last_opp_tick_count = GetTickCount();
			player = COM;
			if (flag.sound) {
				if (!sndPlaySound("commove.wav",SND_ASYNC)) {
					flag.sound = 0;
					sprintf(str,"Error playing sound file. Sound disabled");
					ShowMessage(str);
				}
			}
			UpdateStatusBar("Your turn",0,0);
			hdc = GetDC(mainwin);
			PutTick(hdc, MOVEX(move.move)*tick_size+ST_COORDX, MOVEY(move.move)*tick_size+ST_COORDY, player);
			ReleaseDC(mainwin,hdc);
			MakeMove(move.move, COM);
#ifdef TTABLE
			if (flag.ttable) {
				UpdateHashKeys(move.move,COM);
			}
#endif
			if (flag.quit) {
				PostQuitMessage(0);
				player = OPP;
				continue;
			}
			move_num++;
			player = OPP;
			flag.compmove = FALSE;
			flag.force = FALSE;
	 	}
	 	if ((winner = TestWin()) != NOWIN) {
			player = OPP;
			ShowResult(mainwin, winner,0);
			flag.havewinner = TRUE;
			if (flag.both) {
				flag.both = FALSE;
				TurnOnMenuItems(mainwin);
				CheckMenuItem ( GetMenu(mainwin), IDM_BOTH, MF_UNCHECKED);
			}
	 	}
	 	if (flag.havewinner || GetPieceNbr() == 0)
	 		EnableMenuItem(GetMenu(hWND),IDM_AGRESSIVEMODE,MF_ENABLED);
	 	else
			EnableMenuItem(GetMenu(hWND),IDM_AGRESSIVEMODE,MF_GRAYED);
	 	lastplayer = player;
	}
	EndGame();
	return (msg.wParam);  // Returns the value from PostQuitMessage

}
BOOL InitApplication(HINSTANCE hInstance)
{
	int x_screen_size;
  	WNDCLASS  mainwc;
#ifdef RANDMOVE
	srand(time(0));
#endif
	// Fill the tick square array
	x_screen_size = GetSystemMetrics(SM_CXSCREEN);
	if (x_screen_size < 700) win_type = small;
	else if (x_screen_size >= 700 && x_screen_size <= 950) win_type = medium;
	else win_type = large;

	mainwin_width = (size*((int)win_width[win_type]/size));
	tick_size = TICK_SIZE;
	total_mainwin_width = mainwin_width+122;
	total_mainwin_height = mainwin_width+167;
	InitSqArray();
	player = OPP;

  // Fill in window class structure with parameters that describe the
  // main window.

	mainwc.style = CS_HREDRAW | CS_VREDRAW; // Class style(s).
	mainwc.lpfnWndProc = MainWndProc; // Function to retrieve messages for
					 // windows of this class.
	mainwc.cbClsExtra = 0;  // No per-class extra data.
	mainwc.cbWndExtra = 0;  // No per-window extra data.
	mainwc.hInstance = hInstance; // Application that owns the class.
	mainwc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	mainwc.hCursor = LoadCursor(NULL, IDC_ARROW);
	mainwc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
	mainwc.lpszMenuName = szMenu;  // Name of menu resource in .RC file.
	mainwc.lpszClassName = "Nick's app"; // Name used in call to CreateWindow.

  /* Register the window class and return success/failure code. */

	return (RegisterClass(&mainwc));
}

HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND  hWnd; // Main window handle.

  /* Create a main window for this application instance.  */

	hWnd = CreateWindow(
	"Nick's app",                 // See RegisterClass() call.
	"Windows 5-in-a-row",  // Text for window title bar.
	WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_BORDER ^ WS_CAPTION,
									// Window style.
	100,         					// horizontal position.
	100,          					// Default vertical position.
	total_mainwin_width,       		//  width.
	total_mainwin_height,        	//  height.
	NULL,                  			// Overlapped windows have no parent.
	NULL,                  			// Use the window class menu.
	hInstance,             			// This instance owns this window.
	NULL                   			// Pointer not needed.
 	);

  /* If window could not be created, return "failure" */

	if (!hWnd)
		return (FALSE);

  /* Make the window visible; update its client area; and return "success" */

	ShowWindow(hWnd, nCmdShow); // Show the window
  	UpdateWindow(hWnd);     // Sends WM_PAINT message

	return (hWnd);        // Returns the value from PostQuitMessage
}


#pragma argsused
LRESULT FAR PASCAL MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, temp, temp1, temp2, temp3;
	long int long_temp;
	HDC hdc;
	RECT rect = {0,0,0,0};
	PAINTSTRUCT ps;
	OPENFILENAME ofn;
	FILE *fp;
	int thissize, thisconnects;
	OPENFILENAME ofn2;
	FILE *fp2;
	char fname2[30], str[100], midi_file[128];
	short int *board;
	char fname[30];
	switch (message) {
		case WM_PAINT:
	 		hdc = BeginPaint(hWnd,&ps);
			SetBkMode(hdc,TRANSPARENT);
			drawBoard(hdc);
			EndPaint(hWnd,&ps);
			break;
		case WM_CREATE:
			MainWin_XPos = 100;
			MainWin_YPos = 100;
			hPallete = CreateDialog (hInst, MAKEINTRESOURCE(PALLETE),
			hWnd, (DLGPROC)PalleteDlgProc);
			if (hPallete == NULL) {
				ShowMessage("Error: couldn't load the pallete window");
				exit(1);
			}
			SetWindowPos(hPallete,HWND_TOP,total_mainwin_width+100,100,0,0,SWP_NOSIZE);
			if (flag.timed_game) {
				hClock = CreateDialog (hInst, MAKEINTRESOURCE(CLOCK),
				hWnd, (DLGPROC)ClockDlgProc);
				SetWindowPos(hClock,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218,0,0,SWP_NOSIZE);
			}
			return 0;

	 	case WM_LBUTTONDOWN:
			if (flag.flashing) {
				flag.flashing = 0;
				break;
			}
			if (flag.edit) {
				player = OPP;
				hdc = GetDC(hWnd);
				CheckMove(hdc,LOWORD(lParam),HIWORD(lParam));
				ReleaseDC(hWnd,hdc);
				break;
			}
			if (flag.havewinner || flag.both || flag.compmove)
				break;
			hdc = GetDC(hWnd);
			if (CheckMove(hdc,LOWORD(lParam),HIWORD(lParam))) {
				player = COM;
				if (flag.sound) {
					if (!sndPlaySound("oppmove.wav",SND_ASYNC)) {
						flag.sound = 0;
						sprintf(str,"Error playing sound file. Sound disabled");
						ShowMessage(str);
					}
				}
				if (flag.timed_game && GetPieceNbr() != 1 && !flag.compmove) {
					opp_end = GetTickCount();
					opp_remaining_time-=(opp_end-opp_start);
					SetClockTime(opp_remaining_time,OPP);
				}
			}
			ReleaseDC(hWnd,hdc);
			break;

	 	case WM_RBUTTONDOWN:
			if (flag.edit) {
				player = COM;
				hdc = GetDC(hWnd);
				CheckMove(hdc,LOWORD(lParam),HIWORD(lParam));
				ReleaseDC(hWnd,hdc);
			}
			break;
		case WM_SIZE:
			CreateSBar(hWND,"Ready",5);
			SetWindowPos(hPallete,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos,0,0,SWP_NOSIZE);
			if (flag.timed_game) {
				SetWindowPos(hClock,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218,0,0,SWP_NOSIZE);
				GetWindowRect(hClock,&rect);
			}
			if (flag.post) SetWindowPos(hStats,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218+rect.bottom-rect.top,0,0,SWP_NOSIZE);
			break;
	 	case WM_MOVE:
			SetWindowPos(hPallete,HWND_TOP,(int)LOWORD(lParam)+total_mainwin_width-3,(int)HIWORD(lParam)-41,0,0,SWP_NOSIZE);
			if (flag.timed_game) {
				SetWindowPos(hClock,HWND_TOP,(int)LOWORD(lParam)+total_mainwin_width-3,(int)HIWORD(lParam)+177+rect.bottom-rect.top,0,0,SWP_NOSIZE);				GetWindowRect(hClock,&rect);
			}
			if (flag.post) {
				SetWindowPos(hStats,HWND_TOP,(int)LOWORD(lParam)+total_mainwin_width-3,(int)HIWORD(lParam)+177+rect.bottom-rect.top,0,0,SWP_NOSIZE);
			}
			MainWin_XPos = LOWORD(lParam)-3;
			MainWin_YPos = HIWORD(lParam)-41;
			break;

		case MM_MCINOTIFY:
			if (flag.mididev)
				closeMIDIDevice();

			break;
	 	case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_NEW:
					flag.havewinner = 0;
					EndGame();
					InitGame();
					ClearWindow(hWnd);
					hdc = GetDC(hWnd);
					drawBoard(hdc);
					ReleaseDC(hWnd,hdc);
					break;
				case IDM_QUIT:
					flag.quit = 1;
					flag.both = 0;
					flag.compmove = 0;
					/*SendMessage(hWnd,WM_QUIT,wParam,lParam);*/
					break;

				case IDM_DEPTH:
					lpszTitle="Depth...";
					strcpy(prompt_title,"Search Depth:");
					do {
						temp=DlgInt1=depth;
						if (DialogBox ( hInst, MAKEINTRESOURCE(NUMDLG), hWnd,
							(DLGPROC)GetNumDlgProc)) temp=DlgInt1;
						if (DlgInt1 < 1 || DlgInt1 > 9) {
							ShowMessage("Error: Search depth must be 1-9");
							DlgInt1 = depth;
						}
					} while (temp < 1 || temp > 9);
					depth = temp;
					break;
				case IDM_TIME:
					lpszTitle="Set search time...";
					strcpy(prompt_title,"Maximum response time:");
					do {
						long_temp = search_time;
						DlgLong = search_time;
						if (DialogBox ( hInst, MAKEINTRESOURCE(TIME), hWnd,
							(DLGPROC)TimeDlgProc)) long_temp=DlgLong;
							if (long_temp < 0)
								ShowMessage("Error: Search time must be positive");
					} while (long_temp < 0);
					search_time = long_temp;
					break;
				case IDM_CLOCKTIME:
					lpszTitle="Set clock time...";
					strcpy(prompt_title,"Total game time:");
					do {
						long_temp=game_time;
						DlgLong=game_time;
						if (DialogBox ( hInst, MAKEINTRESOURCE(TIME), hWnd,
							(DLGPROC)TimeDlgProc)) long_temp=DlgLong;
						if (long_temp <= 0) {
							ShowMessage("Error: Total game time must be greater than zero");
						}
					} while (long_temp <= 0);
					opp_remaining_time = long_temp;
					com_remaining_time = long_temp;
					game_time = long_temp;
					SetClockTime(opp_remaining_time,OPP);
					SetClockTime(com_remaining_time,COM);
					break;

				case IDM_AGRESSIVEMODE:
					if (score_shift_connections == 1) {
						score_shift_connections = 2;
						CheckMenuItem(GetMenu(hWnd),IDM_AGRESSIVEMODE,MF_CHECKED);
					}
					else {
						score_shift_connections = 1;
						CheckMenuItem(GetMenu(hWnd),IDM_AGRESSIVEMODE,MF_UNCHECKED);
					}
					if (GetPieceNbr() == 0) {
						EndGame();
						InitGame();
					}
					break;
				case IDM_EXTENSIONS:
					flag.search_extensions ^= 1;
					if (flag.search_extensions) CheckMenuItem(GetMenu(hWnd),IDM_EXTENSIONS,MF_CHECKED);
					else CheckMenuItem(GetMenu(hWnd),IDM_EXTENSIONS,MF_UNCHECKED);
					break;
				case IDM_ITERATIVEDEEPENING:
					flag.iterative_deepening ^= 1;
					CheckMenuItem(GetMenu(hWnd),IDM_ITERATIVEDEEPENING,(flag.iterative_deepening ? MF_CHECKED : MF_UNCHECKED));
					if (!flag.iterative_deepening) {
						CheckMenuItem(GetMenu(hWnd),IDM_TIMEDGAME,MF_UNCHECKED);
						flag.timed_game = 0;
						DestroyWindow(hClock);
						GetWindowRect(hClock,&rect);
						SetWindowPos(hStats,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218+rect.bottom-rect.top,0,0,SWP_NOSIZE);
					}
					break;
				case IDM_TTABLE:
					flag.ttable ^= 1;
					CheckMenuItem(GetMenu(hWnd),IDM_TTABLE,(flag.ttable?MF_CHECKED:MF_UNCHECKED));
					break;
				case IDM_TIMEDGAME:
					flag.timed_game ^= 1;
					CheckMenuItem(GetMenu(hWnd),IDM_TIMEDGAME,(flag.timed_game ? MF_CHECKED : MF_UNCHECKED));
					if (flag.timed_game) {
						flag.iterative_deepening = 1;
						CheckMenuItem(GetMenu(hWnd),IDM_ITERATIVEDEEPENING, MF_CHECKED);
						hClock = CreateDialog (hInst, MAKEINTRESOURCE(CLOCK),
						hWnd, (DLGPROC)ClockDlgProc);
						SetWindowPos(hClock,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218,0,0,SWP_NOSIZE);
						if (flag.post) {
							GetWindowRect(hClock,&rect);
							SetWindowPos(hStats,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218+rect.bottom-rect.top,0,0,SWP_NOSIZE);
						}
					}
					else {
						DestroyWindow(hClock);
						StopTimer();
						if (flag.post)
							SetWindowPos(hStats,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218,0,0,SWP_NOSIZE);

					}
					break;
				case IDM_POST:
					if (!flag.post) {
						lpszTitle="Search Stats";
						hStats = CreateDialog (hInst, MAKEINTRESOURCE(STATS),
							hWnd, (DLGPROC)StatDlgProc);
						if (flag.timed_game) GetWindowRect(hClock,&rect);
						SetWindowPos(hStats,HWND_TOP,MainWin_XPos+total_mainwin_width,MainWin_YPos+218+rect.bottom-rect.top,0,0,SWP_NOSIZE);
						flag.post=TRUE;
					}
					break;
				case IDM_SAVEPV:
					flag.savepv ^= 1;
					CheckMenuItem(GetMenu(hWnd), IDM_SAVEPV, (flag.savepv ? MF_CHECKED : MF_UNCHECKED));
					break;
				case IDM_EDIT:
					EditBoard(hWnd);
					break;
				case IDM_DONE:
					if (DialogBox ( hInst, MAKEINTRESOURCE(TURN), hWnd,
						 (DLGPROC)GetTurnDlgProc)) {
							if (DlgTurnC) player = COM;
							else player = OPP;
					}
					break;
				case IDM_CLEAR:
					EndGame();
					InitGame();
					ClearWindow(hWnd);
					hdc = GetDC(hWnd);
					drawBoard(hdc);
					ReleaseDC(hWnd,hdc);
					break;
				case IDM_UNDO:
					if (flag.compmove || flag.both) break;
					UnmakeMove();
					ClearWindow(hWnd);
					hdc = GetDC(hWnd);
					drawBoard(hdc);
					ReleaseDC(hWnd,hdc);
					if (flag.havewinner) flag.havewinner = 0;
					if (flag.timed_game) {
						SetClockTime(opp_remaining_time,OPP);
						SetClockTime(com_remaining_time,COM);
					}
					break;
				case IDM_ABOUT:
					DialogBox( hInst, MAKEINTRESOURCE(ABOUT), hWnd,
					 (DLGPROC)AboutDlgProc);
					break;
				case IDM_VERSION:
					DialogBox( hInst, MAKEINTRESOURCE(VERSION), hWnd,
					 (DLGPROC)VersionDlgProc);
					break;
				case IDM_FORCE:
					if (flag.compmove || flag.both)
						flag.force = 1;
					else if (player == OPP) player = COM;
					break;
				case IDM_BOTH:
					if (flag.havewinner) break;
					if (!flag.both) {
						if (flag.timed_game) {
							lpszTitle="Set Clock time";
							for (i=0;i<2;i++) {
								do {
									long_temp = game_time;
									DlgLong = game_time;
									strcpy(prompt_title,(i==0?"Computer 1 clock time:":"Computer 2 clock time"));
									if (DialogBox ( hInst, MAKEINTRESOURCE(TIME), hWnd,
									(DLGPROC)TimeDlgProc)) long_temp=DlgLong;
									if (long_temp <= 0) {
										ShowMessage("Error: Total game time must be greater than zero");
									}
								} while (long_temp <= 0);
								if (i==0) com_remaining_time = long_temp;
								else opp_remaining_time = long_temp;
							}
							game_time = com_remaining_time;
							SetClockTime(com_remaining_time,COM);
							SetClockTime(opp_remaining_time,OPP);
						}

						else {
            				lpszTitle="Search depth";
							for (i=0;i<2;i++) {
								do {
									temp=DlgInt1=DEFAULT_DEPTH;
									strcpy(prompt_title,(i == 0 ? "Computer 1 depth:" : "Computer 2 depth:"));
									if (DialogBox ( hInst, MAKEINTRESOURCE(NUMDLG), hWnd,
										(DLGPROC)GetNumDlgProc)) temp=DlgInt1;
									else return 0;
									if (DlgInt1 < 1 || DlgInt1 > 9) {
										ShowMessage("Error: Search depth must be 1-9");
										DlgInt1 = DEFAULT_DEPTH;
									}
								} while (temp < 1 || temp > 9);
								if (i == 0) depth = temp;
					   			else depth2 = temp;
							}
						}
						flag.both = TRUE;
						CheckMenuItem ( GetMenu(hWnd), IDM_BOTH, MF_CHECKED);
						TurnOffMenuItems(hWnd);
					}
					else {
						flag.both = FALSE;
						flag.force = 1;
						CheckMenuItem ( GetMenu(hWnd), IDM_BOTH, MF_UNCHECKED);
						TurnOnMenuItems(hWnd);
					}
					player = COM;
					break;

				case IDM_FLASH:
					if (!flag.flash) {
						flag.flash = 1;
						CheckMenuItem(GetMenu(hWnd),IDM_FLASH,MF_CHECKED);
						EnableMenuItem(GetMenu(hWnd),IDM_FLASHDELAY,MF_ENABLED);
					}
					else {
						flag.flash = 0;
						CheckMenuItem(GetMenu(hWnd),IDM_FLASH,MF_UNCHECKED);
						EnableMenuItem(GetMenu(hWnd),IDM_FLASHDELAY,MF_GRAYED);
					}
					break;
				case IDM_FLASHDELAY:
					lpszTitle="Flash delay...";
					strcpy(prompt_title,"Milisecs/flash:");
					do {
						temp=DlgInt1=flash_delay;
						if (DialogBox ( hInst, MAKEINTRESOURCE(NUMDLG), hWnd,
							 (DLGPROC)GetNumDlgProc)) temp=DlgInt1;
						if (temp >= 2000 || temp <= 10) {
							ShowMessage("Error: Delay must be 10-2000 msecs");
							DlgInt1 = flash_delay;
						}
					} while (temp > 2000 || temp < 10);
					flash_delay = temp;
					break;

				case IDM_SOUND:
					flag.sound ^= 1;
					if (flag.sound)
						CheckMenuItem(GetMenu(hWnd),IDM_SOUND,MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd),IDM_SOUND,MF_UNCHECKED);
					break;
				case IDM_STYLE:
					flag.newticks ^= 1;
					CheckMenuItem(GetMenu(hWnd),IDM_STYLE,(flag.newticks ? MF_CHECKED : MF_UNCHECKED));
					ClearWindow(hWnd);
					hdc = GetDC(hWnd);
					drawBoard(hdc);
					ReleaseDC(hWnd,hdc);
					break;
				case IDM_PROPS:
         			lpszTitle = "Game properties...";
					temp1 = DlgInt1 = size;
					temp2 = DlgInt2 = connects;
					temp3 = flag.standard_gomoku;
					if (DialogBox ( hInst, MAKEINTRESOURCE(PROPS), hWnd,
						(DLGPROC)PropsDlgProc)) {
					}
					if (temp1 != DlgInt1 || temp2 != DlgInt2 || temp3 != flag.standard_gomoku) {
						EndGame();
						size=DlgInt1;
						connects=DlgInt2;
						mainwin_width = (size*((int)win_width[win_type]/size));
						tick_size = TICK_SIZE;
						InitSqArray();
						flag.havewinner = 0;
						if (size != 19) {
							EnableMenuItem(GetMenu(hWND),IDM_STANDARD,MF_GRAYED);
							CheckMenuItem(GetMenu(hWnd),IDM_STANDARD,MF_UNCHECKED);
							flag.standard_gomoku = 0;
						}
						else
							EnableMenuItem(GetMenu(hWND),IDM_STANDARD,MF_ENABLED);
						InitGame();
						ClearWindow(hWnd);
						hdc = GetDC(hWnd);
						drawBoard(hdc);
						ReleaseDC(hWnd,hdc);

					}

					break;
				case IDM_SMALL:
					DestroyStatusBar();
					win_type = small;
					CheckMenuItem(GetMenu(hWnd),IDM_SMALL,MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_MEDIUM,MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_LARGE,MF_UNCHECKED);
					mainwin_width = (size*((int)win_width[win_type]/size));
					tick_size = TICK_SIZE;
					total_mainwin_width = mainwin_width+122;
					total_mainwin_height = mainwin_width+167;
					InitSqArray();
					MoveWindow(hWnd,100,100,total_mainwin_width,total_mainwin_height,TRUE);
					break;

				case IDM_MEDIUM:
					DestroyStatusBar();
					win_type = medium;
					CheckMenuItem(GetMenu(hWnd),IDM_SMALL,MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_MEDIUM,MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_LARGE,MF_UNCHECKED);
					mainwin_width = (size*((int)win_width[win_type]/size));
					tick_size = TICK_SIZE;
					total_mainwin_width = mainwin_width+122;
					total_mainwin_height = mainwin_width+167;
					InitSqArray();
					MoveWindow(hWnd,100,100,total_mainwin_width,total_mainwin_height,TRUE);
					break;

				case IDM_LARGE:
					DestroyStatusBar();
					win_type = large;
					CheckMenuItem(GetMenu(hWnd),IDM_SMALL,MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_MEDIUM,MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd),IDM_LARGE,MF_CHECKED);
					mainwin_width = (size*((int)win_width[win_type]/size));
					tick_size = TICK_SIZE;
					total_mainwin_width = mainwin_width+122;
					total_mainwin_height = mainwin_width+167;
					InitSqArray();
					MoveWindow(hWnd,100,100,total_mainwin_width,total_mainwin_height,TRUE);
					break;

				case IDM_MUSIC:
					if (flag.music) {
						flag.music = 0;
						closeMIDIDevice();
						CheckMenuItem(GetMenu(hWnd),IDM_MUSIC,MF_UNCHECKED);
					}
					else
						if (getMIDIFileName(midi_file)) {
							closeMIDIDevice();
							flag.music = 1;
							playMIDIFile(hWnd,midi_file);
							CheckMenuItem(GetMenu(hWnd),IDM_MUSIC,MF_CHECKED);
						}
					break;

				case IDM_NEWMUSIC:
					CheckMenuItem(GetMenu(hWnd),IDM_MUSIC,MF_CHECKED);
					closeMIDIDevice();
					if (getMIDIFileName(midi_file)) {
						playMIDIFile(hWnd,midi_file);
						flag.music = 1;
					}
					break;

				case IDM_SAVE:
					fname[0]='\0';
					memset(&ofn,0,sizeof(ofn));
					ofn.hwndOwner=hWnd;
					ofn.lpstrFilter=filter;
					ofn.nFilterIndex=1;
					ofn.lpstrFile=fname;
					ofn.nMaxFile=256;
					ofn.lpstrTitle="Save Game";
#ifdef WIN32
					ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER;
#else
					ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
#endif
					ofn.lStructSize=sizeof(ofn);

					if (GetSaveFileName(&ofn)) {
						board = (short int *)malloc(size * size * sizeof(short int ));
						for (i=0;i<TOTALSQS;i++) {
							board[i] = (short int)GetSquare(i);
						}

						fp = fopen(fname, "wb");
						fwrite(&size, sizeof(size), 1, fp);
						fwrite(&connects, sizeof(connects), 1, fp);
						fwrite(board, sizeof(short int), size*size, fp);
						fwrite(&flag.standard_gomoku, sizeof(flag.standard_gomoku), 1, fp);
						fclose(fp);
						free(board);
					}
					break;

				case IDM_GET:
					fname2[0]='\0';
					memset(&ofn2,0,sizeof(ofn2));
					ofn2.hwndOwner=hWnd;
					ofn2.lpstrFilter=filter;
					ofn2.nFilterIndex=1;
					ofn2.lpstrFile=fname2;
					ofn2.nMaxFile=256;
					ofn2.lpstrTitle="Open Game";
#ifdef WIN32
					ofn2.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER;
#else
					ofn2.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
#endif
					ofn2.lStructSize=sizeof(ofn2);

					if (GetOpenFileName(&ofn2)) {
						if ((fp2 = fopen(fname2, "rb")) == NULL) {
							ShowMessage("File inexistent");
							break;
						}
						fread(&thissize, sizeof(size), 1, fp2);
						fread(&thisconnects, sizeof(connects), 1, fp2);
						board = (short int *)malloc(thissize*thissize * sizeof(short int));
						fread(board, sizeof(short int), thissize*thissize, fp2);
						fread(&flag.standard_gomoku, sizeof(flag.standard_gomoku), 1, fp2);
						fclose(fp2);
						EndGame();
						size = thissize;
						connects = thisconnects;
						mainwin_width = (size*((int)win_width[win_type]/size));
						tick_size = TICK_SIZE;
						InitSqArray();
						InitGame();
						for (i=0;i<TOTALSQS;i++)
							if (board[i] != EMPTY)
								MakeMove(i, board[i]);
						free(board);
						flag.havewinner = 0;
						ClearWindow(hWnd);
						hdc = GetDC(hWnd);
						drawBoard(hdc);
						ReleaseDC(hWnd,hdc);
						if (flag.timed_game) {
							last_com_tick_count = last_opp_tick_count = GetTickCount();
						}
						sprintf(str,"The board's size is %dx%d squares.\nYou have to connect %d ticks in a row to win", thissize, thissize, thisconnects);
						MessageBox(hWnd,str,"Windows 5-in-a-row",MB_OK|MB_ICONINFORMATION);
						if (flag.timed_game) opp_start = GetTickCount();
						break;
					}
					break;

				case IDM_INDEX:
					if (!WinHelp(hWnd, "helpfile.hlp", HELP_CONTENTS, 0L))
						ShowMessage("Could not open helpfile.hlp");
					break;
				case IDM_HELP:
					if (!WinHelp(hWnd,"winhelp.hlp",HELP_HELPONHELP,0L))
						ShowMessage("Could not open winhelp.hlp");
          			break;
			}
			break;

	 	case WM_QUIT:
	 	case WM_DESTROY:
			flag.quit = 1;
			flag.both = 0;
			flag.compmove = 0;
			if (flag.mididev) closeMIDIDevice();
			DeleteObject(hImage);
			// message: window being destroyed
			break;

	 	default:      // Passes it on if unproccessed
			return (DefWindowProc(hWnd, message, wParam, lParam));
	}
  	return 0;
}

int CheckMove(HDC hdc, int x, int y)	/* Check for a valid opponent move */
{
	int i;
	for (i=0;i<TOTALSQS;i++) {
		if (x > pressedsq[i].lowestx && x < pressedsq[i].highestx &&
			y > pressedsq[i].lowesty && y < pressedsq[i].highesty &&
			IsEmpty(i) && (LegalMove(i) || flag.edit)) {
				PutTick(hdc, pressedsq[i].lowestx, pressedsq[i].lowesty, player);
				MakeMove(i,player);
#ifdef TTABLE
				if (flag.ttable) {
					UpdateHashKeys(i,OPP);
				}
#endif
				if (flag.timed_game && !flag.edit) {
					if (player == OPP) last_com_tick_count = GetTickCount();
					else last_opp_tick_count = GetTickCount();
				}
				return (1);
		}
	}
	return (0);
}

void PutTick(HDC hdc, int x, int y, int turn)   /* Draw a tick in x, y coordinates */
{
	int i;
	DWORD time;
	RECT rect;
	if (turn == OPP) {
		DrawTick(hdc,x,y,OPP);
	}
	else {
		/*Ellipse(hdc, x+2, y+2, x+tick_size-1, y+tick_size-1);*/
		DrawTick(hdc,x,y,COM);
		if (flag.flash && !flag.both) {
			flag.flashing = 1;
			rect.left=x+1;
			rect.top=y+1;
			rect.right=x+tick_size-1;
			rect.bottom=y+tick_size-1;
			for (i=0;i<3;i++) {
				time = GetCurrentTime();
				while (GetCurrentTime()-time < flash_delay*(i+1) && flag.flashing)
					CheckAndLeaveMessage();
				/*FillRect(hdc,&rect,GetStockObject(LTGRAY_BRUSH));*/
				DrawPattern(hdc,x,y);
				while (GetCurrentTime()-time < 2*flash_delay*(i+1) && flag.flashing)
					CheckAndLeaveMessage();
				DrawTick(hdc,x,y,COM);
			}
			flag.flashing = 0;
		}
	}

}
void DrawPattern(HDC hdc, int x, int y)
{
	HDC hdcCompatible;
	hdcCompatible = CreateCompatibleDC(hdc);
	SelectObject(hdcCompatible,hImage);
	BitBlt(hdc,x+2,y+2,tick_size-2,tick_size-2,hdcCompatible,0,0,SRCCOPY);
	DeleteDC(hdcCompatible);
}

void DrawTick(HDC hdc, int x, int y, int player)
{
	HPEN hPen1, hPen2, hOldPen;
	HBRUSH hBrush1, hBrush2, hOldBrush;
	switch (player) {
		case OPP:
			if (flag.newticks) {
				hPen1 = CreatePen(PS_SOLID,3,RGB(3,3,3));
				hOldPen = SelectObject(hdc,hPen1);
				drawLine(hdc, x+4, y+4, tick_size-8, tick_size-8);
				drawLine(hdc, x+4, y+tick_size-4, tick_size-8, -tick_size+8);
				SelectObject(hdc,hOldPen);
				DeleteObject(hPen1);
			}
			else {
				drawLine(hdc, x, y, tick_size, tick_size);
				drawLine(hdc, x, y+tick_size, tick_size, -tick_size);
			}
			break;
		case COM:
			if (flag.newticks) {
				hBrush1 = CreateSolidBrush(RGB(40,100,60));
				hBrush2 = CreateSolidBrush(RGB(3,3,3));
				hPen1 = CreatePen(PS_SOLID,1,RGB(40,100,60));
				hPen2 = CreatePen(PS_SOLID,1,RGB(3,3,3));
				hOldBrush = SelectObject(hdc, hBrush2);
				hOldPen = SelectObject(hdc, hPen2);
				Ellipse(hdc, x+2, y+2, x+tick_size-1, y+tick_size-1);
				SelectObject(hdc, hBrush1);
				SelectObject(hdc, hPen1);
				Ellipse(hdc, x+4, y+4, x+tick_size-1, y+tick_size-1);
				SelectObject(hdc, hOldPen);
				SelectObject(hdc, hOldBrush);
				DeleteObject(hPen1);
				DeleteObject(hPen2);
				DeleteObject(hBrush1);
				DeleteObject(hBrush2);
			}
			else {
				Ellipse(hdc, x+2, y+2, x+tick_size-1, y+tick_size-1);
			}
			break;
		default:
			break;
	}
}

void drawBoard(HDC hdc)			/* Draw the board */
{
	int i, j, pl, temp;
	char str[3];
	RECT lprc;
	HPEN pen, old_pen;
	HBRUSH old_brush;
	HDC hdcCompatible;

	hdcCompatible = CreateCompatibleDC(hdc);
	SelectObject(hdcCompatible,hImage);
	for (i=ST_COORDX;i<MAINWIN_WIDTH+160;i+=160) for(j=ST_COORDY;j<MAINWIN_WIDTH+160;j+=160) {
		BitBlt(hdc,i,j,160,160,hdcCompatible,0,0,SRCCOPY);
	}
	DeleteDC(hdcCompatible);
	lprc.left = ST_COORDX+MAINWIN_WIDTH;
	lprc.top = 0;
	lprc.right = TOTAL_MAINWIN_WIDTH;
	lprc.bottom = TOTAL_MAINWIN_HEIGHT+100;
	FillRect(hdc,&lprc,GetStockObject(LTGRAY_BRUSH));
	lprc.left = 0;
	lprc.top = ST_COORDY+MAINWIN_WIDTH;
	lprc.right = TOTAL_MAINWIN_WIDTH;
	lprc.bottom = TOTAL_MAINWIN_HEIGHT+100;
	FillRect(hdc,&lprc,GetStockObject(LTGRAY_BRUSH));

	lprc.left = 10; lprc.top = 10; lprc.right = total_mainwin_width-20; lprc.bottom = total_mainwin_height-80;
	DrawFocusRect(hdc,&lprc);
	temp = flag.flash;
	flag.flash = 0;
	pen = CreatePen(PS_SOLID,2,RGB(225,225,225));
	for (i=0;i<=SIZE;i++) {
		drawLine(hdc, ST_COORDX+i*tick_size, ST_COORDY, 0, MAINWIN_WIDTH);
		old_pen = SelectObject(hdc,pen);
		drawLine(hdc, ST_COORDX+i*tick_size+2, ST_COORDY, 0, MAINWIN_WIDTH);
		SelectObject(hdc,old_pen);
	}
	for (i=0;i<=SIZE;i++) {
		drawLine(hdc, ST_COORDX, ST_COORDY+i*tick_size, MAINWIN_WIDTH, 0);
		old_pen = SelectObject(hdc,pen);
		drawLine(hdc, ST_COORDX, ST_COORDY+i*tick_size+2, MAINWIN_WIDTH,0);
		SelectObject(hdc,old_pen);
	}
	DeleteObject(pen);

	if (flag.standard_gomoku) {
		pen = CreatePen(PS_SOLID,0,RGB(10,10,250));
		old_pen = SelectObject(hdc,pen);
		old_brush = SelectObject(hdc,GetStockObject(LTGRAY_BRUSH));
		drawLine(hdc,ST_COORDX+7*tick_size,ST_COORDY+7*tick_size,5*tick_size,0);
		drawLine(hdc,ST_COORDX+7*tick_size,ST_COORDY+7*tick_size,0,5*tick_size);
		drawLine(hdc,ST_COORDX+7*tick_size,ST_COORDY+12*tick_size,5*tick_size,0);
		drawLine(hdc,ST_COORDX+12*tick_size,ST_COORDY+7*tick_size,0,5*tick_size);
		Rectangle(hdc,ST_COORDX+9*tick_size,ST_COORDY+9*tick_size,ST_COORDX+10*tick_size+1,ST_COORDY+10*tick_size+1);
		DrawPattern(hdc,ST_COORDX+9*tick_size,ST_COORDY+9*tick_size);
		SelectObject(hdc,old_pen);
		SelectObject(hdc,old_brush);
		DeleteObject(pen);
	}

	pen = CreatePen(PS_SOLID,3,RGB(220,220,220));
	old_pen = SelectObject(hdc,pen);
	drawLine(hdc,ST_COORDX-1,ST_COORDY-1,MAINWIN_WIDTH,0);
	drawLine(hdc,ST_COORDX-1,ST_COORDY-1,0,MAINWIN_WIDTH);
	DeleteObject(pen);
	pen = CreatePen(PS_SOLID,3,RGB(120,120,120));
	SelectObject(hdc,pen);
	drawLine(hdc,ST_COORDX,ST_COORDY+MAINWIN_WIDTH,MAINWIN_WIDTH,0);
	drawLine(hdc,ST_COORDX+MAINWIN_WIDTH,ST_COORDY,0,MAINWIN_WIDTH);
	SelectObject(hdc,old_pen);
	DeleteObject(pen);
	drawLine(hdc,ST_COORDX,ST_COORDY+1,MAINWIN_WIDTH-4,0);
	drawLine(hdc,ST_COORDX+1,ST_COORDY,0,MAINWIN_WIDTH-4);
	for (i=0;i<TOTALSQS;i++)
		if ((pl = GetSquare(i)) != EMPTY)
			PutTick(hdc, MOVEX(i)*tick_size+ST_COORDX, MOVEY(i)*tick_size+ST_COORDY, pl);
	SetBkMode(hdc,TRANSPARENT);
	for (i=0;i<size;i++) {
		sprintf(str,"%c",'A'+i);
		TextOut(hdc,ST_COORDX+i*TICK_SIZE+TICK_SIZE/2-5,ST_COORDY+mainwin_width+10,str,strlen(str));
		if (i < 10) sprintf(str,"%d",size-i);
		else sprintf(str," %d",size-i);
		TextOut(hdc,ST_COORDX-25,ST_COORDY+i*TICK_SIZE+TICK_SIZE/2-5,str,strlen(str));
	}

	flag.flash = temp;
}

void drawLine(HDC hdc, int x, int y, int width, int length)
{
	POINT points[2];

	points[0].x = x; points[0].y = y;
	points[1].x = x+width; points[1].y = y+length;
	Polygon(hdc, points, 2);
}

int ShowIntroMessages(HWND hWnd)
{
    char str[128];

	 {
		sprintf(str, "Welcome to Windows 5-in-a-row!!\nNick's ultimate game.\nHave fun!");
		MessageBox(hWnd,str,"Windows 5-in-a-row",MB_OK|MB_ICONINFORMATION);
	 }

	 return 0;
}

void ShowResult(HWND hWnd, int winner, int win_on_time)
{
	char str[256], sound_file[30];
	int x1,x2,y1,y2;
	HDC hdc;
	HPEN hPen, hOldPen;

	if ((winner == OPPWINS || winner == COMWINS) && !win_on_time) {
		GetWinCoords(&x1,&y1,&x2,&y2);
		hdc = GetDC(hWnd);
		hPen = CreatePen(PS_SOLID,3,RGB(100,0,0));
		hOldPen = SelectObject(hdc,hPen);
		drawLine(hdc,ST_COORDX+TICK_SIZE*x1+TICK_SIZE/2,ST_COORDY+TICK_SIZE*y1+TICK_SIZE/2,TICK_SIZE*(x2-x1),TICK_SIZE*(y2-y1));
		SelectObject(hdc,hPen);
		ReleaseDC(hWnd,hdc);
	}
	switch (winner) {
		case OPPWINS:
			sprintf(sound_file, "youwin.wav");
			if (flag.both) {
				sprintf(str, "Computer 2 wins%s\n(Search Depth: %d vs %d)",(win_on_time?" on time":""),depth, depth2);
				break;
			}
			sprintf(str, "Congratulations!!\nYou win%s. :)",(win_on_time?" on time":""));
			break;
		case COMWINS:
			if (flag.both) {
				sprintf(sound_file, "youwin.wav");
				sprintf(str, "Computer 1 wins%s\n(Search Depth: %d vs %d)",(win_on_time?" on time":""),depth, depth2);
				break;
			}
			sprintf(sound_file, "youlose.wav");
			sprintf(str, "Oooooh, I win%s :(",(win_on_time?" on time":""));
			break;
		case DRAW:
			sprintf(sound_file, "draw.wav");
			if (flag.both) {
				sprintf(str, "The game was drawn.");
				break;
			}
			sprintf(str, "The game is a draw. Nice playing.");
			break;
		default:
			return;
	}
	if (flag.sound)
		if (!sndPlaySound(sound_file,SND_ASYNC)) {
			sprintf(sound_file,"Error playing sound file. Sound disabled.");
			flag.sound = 0;
			ShowMessage(sound_file);
		}
	MessageBox(hWnd,str,"Windows 5-in-a-row result",MB_OK|MB_ICONINFORMATION);
}

void ClearWindow(HWND hWnd)
{
	HDC hdc;
	RECT rect;
	HBRUSH hbrush;
	hdc = GetDC(hWnd);
	hbrush = CreateSolidBrush(RGB(192,192,192));
	rect.left = 0; rect.top = 0; rect.right = TOTAL_MAINWIN_WIDTH; rect.bottom = TOTAL_MAINWIN_HEIGHT;
	FillRect(hdc, &rect, hbrush);

	ReleaseDC(hWnd,hdc);
}

void
ShowMessage (char *s)
{
  MessageBox(mainwin,s,"message",MB_OK);
}

void ShowMessageFromWin (char *s, HWND hWnd)
{
	MessageBox(hWnd,s,"message",MB_OK);
}


void EditBoard(HWND hWnd)
{
	HMENU hMainMenu,hEditMenu;
    int temp;
	flag.edit = TRUE;

	temp = flag.flash;
	flag.flash = FALSE;
	hEditMenu=LoadMenu(hInst,"EDIT");
	hMainMenu=GetMenu(hWnd);
	SetMenu(hWnd,hEditMenu);
	while (flag.edit) WaitForMessage();

	SetMenu(hWnd,hMainMenu);
	DestroyMenu(hEditMenu);
	flag.flash = temp;
}

void CheckMessage(void)
{
  MSG msg;

  if ( PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	 if ( !TranslateAccelerator (hWND, hAccel, &msg) )
	  {    TranslateMessage(&msg);
		 DispatchMessage(&msg); }
}

void CheckAndLeaveMessage(void)
{
	MSG msg;

  	if ( PeekMessage(&msg, 0, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE))
	 	if ( !TranslateAccelerator (hWND, hAccel, &msg) ) {
			TranslateMessage(&msg);
		 	DispatchMessage(&msg);
		}
}

void WaitForMessage(void)
{
  MSG msg;

  if ( GetMessage(&msg, 0, 0, 0))
	 if ( !TranslateAccelerator (hWND, hAccel, &msg) )
	  {    TranslateMessage(&msg);
		 DispatchMessage(&msg); }
}

void TurnOffMenuItems(HWND hWnd)
{
	EnableMenuItem(GetMenu(hWnd),IDM_NEW,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_DEPTH,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_NEW,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_EDIT,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_ABOUT,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_SAVE,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_GET,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_PROPS,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_POST,MF_GRAYED);
	if (!flag.both) EnableMenuItem(GetMenu(hWnd),IDM_BOTH,MF_GRAYED);
	/*EnableMenuItem(GetMenu(hWnd),IDM_FORCE,MF_ENABLED);*/
	EnableMenuItem(GetMenu(hWnd),IDM_FLASH,MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd),IDM_FLASHDELAY,MF_GRAYED);
}

void TurnOnMenuItems(HWND hWnd)
{
	EnableMenuItem(GetMenu(hWnd),IDM_NEW,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_DEPTH,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_NEW,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_EDIT,MF_ENABLED);
   	EnableMenuItem(GetMenu(hWnd),IDM_ABOUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_SAVE,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_GET,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_PROPS,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_BOTH,MF_ENABLED);
	/*EnableMenuItem(GetMenu(hWnd),IDM_FORCE,MF_GRAYED);*/
	EnableMenuItem(GetMenu(hWnd),IDM_POST,MF_ENABLED);
	EnableMenuItem(GetMenu(hWnd),IDM_FLASH,MF_ENABLED);
	if (flag.flash) EnableMenuItem(GetMenu(hWnd),IDM_FLASHDELAY,MF_ENABLED);
}

void InitSqArray(void)
{
	int i, j;
	for (i=0;i<SIZE;i++) for (j=0;j<SIZE;j++) {
		pressedsq[i*SIZE+j].lowestx = j*tick_size+ST_COORDX;
		pressedsq[i*SIZE+j].highestx = j*tick_size+(tick_size-1)+ST_COORDX;
		pressedsq[i*SIZE+j].lowesty = i*tick_size+ST_COORDY;
		pressedsq[i*SIZE+j].highesty = i*tick_size+(tick_size-1)+ST_COORDY;
	}
}

void InitializeStatusBar(HWND hwndParent,int nrOfParts)
{
    const int cSpaceInBetween = 8;
    int   ptArray[40];   // Array defining the number of parts/sections
    int i;
	RECT  rect;
    HDC   hDC;

   /* * Fill in the ptArray...  */

    hDC = GetDC(hwndParent);
    GetClientRect(hwndParent, &rect);

    ptArray[nrOfParts-1] = rect.right;
	for (i=0;i<nrOfParts-1;i++) {
		ptArray[i] = rect.right-(nrOfParts-i-1)*(rect.right-rect.left)/nrOfParts;
	}
    ReleaseDC(hwndParent, hDC);
    SendMessage(hWndStatusbar,
                SB_SETPARTS,
                nrOfParts,
                (LPARAM)(LPINT)ptArray);

    UpdateStatusBar("Ready", 0, 0);
	UpdateStatusBar("Hello!", 4, 0);

}


/*------------------------------------------------------------------------
 Procedure:     CreateSBar ID:1
 Purpose:       Calls CreateStatusWindow to create the status bar
 Input:         hwndParent: the parent window
                initial text: the initial contents of the status bar
 Output:
 Errors:
------------------------------------------------------------------------*/
static BOOL CreateSBar(HWND hwndParent,char *initialText,int nrOfParts)
{
    hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER/*|SBARS_SIZEGRIP*/,
                                       initialText,
                                       hwndParent,
                                       IDM_STATUSBAR);
    if(hWndStatusbar)
    {
        InitializeStatusBar(hwndParent,nrOfParts);
        return TRUE;
    }

    return FALSE;
}

void UpdateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags)
{
    SendMessage(hWndStatusbar,
                SB_SETTEXT,
                partNumber | displayFlags,
                (LPARAM)lpszStatusString);
}

void DestroyStatusBar(void)
{
	DestroyWindow(hWndStatusbar);
}

DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName)
{

    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;

	flag.mididev = 1;
    /*
     * Open the device by specifying the
     * device name and device element.
     * MCI will attempt to choose the
     * MIDI Mapper as the output port.

     */
    /*mciOpenParms.lpstrDeviceType = "sequencer";*/
    mciOpenParms.lpstrElementName = lpszMIDIFileName;
    if (dwReturn = mciSendCommand(0, MCI_OPEN,
            /*MCI_OPEN_TYPE | */MCI_OPEN_ELEMENT,
            (DWORD)(LPVOID) &mciOpenParms)) {
        /*
         * Failed to open device;
         * don't close it, just return error.
         */
        return dwReturn;
    }

    /* Device opened successfully. Get the device ID. */
    wDeviceID = mciOpenParms.wDeviceID;

    /* See if the output port is the MIDI Mapper. */


    /*
     * Begin playback. The window procedure function
     * for the parent window is notified with an
     * MM_MCINOTIFY message when playback is complete.
     * The window procedure then closes the device.
     */
    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY,
            MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
        return dwReturn;
    }

    return 0;
}

void closeMIDIDevice(void)
{
	flag.mididev = 0;
	mciSendCommand(wDeviceID,MCI_CLOSE,0,0);
}

int getMIDIFileName(char *file_name)
{
	OPENFILENAME ofn;
	char fname[128], midi_filter[] = "Midi files (*.mid;*.rmi)\0*.mid;*.rmi\0All files (*.*)\0*.*\0";
	fname[0]='\0';
	memset(&ofn,0,sizeof(ofn));
	ofn.hwndOwner=mainwin;
	ofn.lpstrFilter=midi_filter;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=fname;
	ofn.nMaxFile=256;
	ofn.lpstrTitle="Open Midi File";
#ifdef WIN32
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER;
#else
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
#endif
	ofn.lStructSize=sizeof(ofn);

	if (!GetOpenFileName(&ofn)) return (0);

	strcpy(file_name,fname);
	return (1);
}

void StartTimer(void)
{
	idTimer = SetTimer(mainwin,1,1000,ProcessTimer);
	if (idTimer == 0) {
		ShowMessage("Error: couldn't start timer");
		exit(1);
	}
}

void StopTimer(void)
{
	KillTimer(mainwin,idTimer);
}

void ProcessTimer(void)
{
	long int this_tick_count;
	/*ShowMessage("At timer");*/
	if (flag.havewinner || flag.edit || !flag.timed_game || flag.opening_game) return;
	if (GetPieceNbr() == 0) {
		if (player == OPP) last_com_tick_count = GetTickCount();
		else last_opp_tick_count = GetTickCount();
		return;
	}
	switch (player) {
		case OPP:
			this_tick_count = GetTickCount();
			/*opp_remaining_time-=(this_tick_count-last_opp_tick_count);*/
			SetClockTime(opp_remaining_time-this_tick_count+last_opp_tick_count,OPP);
			/*last_opp_tick_count = this_tick_count;*/
			if (opp_remaining_time-this_tick_count+last_opp_tick_count <= 0) {
				StopTimer();
				Flag(OPP);
				ShowResult(mainwin,COMWINS,1);
			}
			break;
		case COM:
			this_tick_count = GetTickCount();
			/*com_remaining_time-=(this_tick_count-last_com_tick_count);*/
			SetClockTime(com_remaining_time-this_tick_count+last_com_tick_count,COM);
			/*last_com_tick_count = this_tick_count;*/
			if (com_remaining_time-this_tick_count+last_com_tick_count <= 0) {
				StopTimer();
				Flag(COM);
				ShowResult(mainwin,OPPWINS,1);
			}
			break;
		default:
			break;
	}
}
