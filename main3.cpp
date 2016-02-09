#include <windows.h>
#include "resource.h"

//bitmap variable declaration
HBITMAP g_hbmKnight = NULL;
HBITMAP g_hbmMask = NULL;
HBITMAP	g_hbmLight = NULL;
HBITMAP g_hbmDark = NULL;
HBITMAP g_hbmFlag = NULL;
HBITMAP g_hbmFlagMask = NULL;

//grid x size, y size and chess board tile width
int sizeX = 8;
int sizeY = 8;
int tileWidth = 75;

//knight position variable declaration
int knightX = 0;
int knightY = 0;

bool tourStarted = false;
bool drawKnight = false;
int *flagArray = new int[sizeX*sizeY];
int totalMoves = 0;

int ups = 500;
const int ID_TIMER = 1;

HWND hwnd;

//define a structure for the knight
typedef struct _KNIGHTINFO
{
	//width and height of the sprite
	int width, height;
	//x and y position on the board
	int x, y;
}KNIGHTINFO;

// defines a structure for chess moves
typedef struct chess_moves {
   //x and y position on the board
   int x,y;
}chess_moves;

KNIGHTINFO g_knightInfo;
chess_moves moveArray[8] = { {2,1}, {1,2},{-1,2},{-2,1}, {-2,-1},{-1,-2},{1,-2},{2,-1} };

void Resize(int newSize, HWND hwnd)
{
	if(!tourStarted){
		sizeX = newSize;
		sizeY = newSize;
		flagArray = new int[sizeX*sizeY];
		SetWindowPos(hwnd, NULL, 10, 10, (sizeX*tileWidth)+10, (sizeY*tileWidth)+52, NULL);
	}	
}

//function to create bitmap masks for transparency
//source: forgers win32 tutorial
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;

	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	SelectObject(hdcMem, hbmColour);
	SelectObject(hdcMem2, hbmMask);

	SetBkColor(hdcMem, crTransparent);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}

void DrawFlag(HDC hdc, HDC hdcMem, int i, int j)
{
	SelectObject(hdcMem, g_hbmFlagMask);
	BitBlt(hdc, i*tileWidth, j*tileWidth, tileWidth, tileWidth, hdcMem, 0, 0, SRCAND);

	SelectObject(hdcMem, g_hbmFlag);
	BitBlt(hdc, i*tileWidth, j*tileWidth, tileWidth, tileWidth, hdcMem, 0, 0, SRCPAINT);
}

void DrawTile(HDC hdc, HDC hdcMem, int i,int j, BITMAP bm, HBITMAP hbm)
{
	SelectObject(hdcMem, hbm);
	GetObject(g_hbmLight, sizeof(bm), &bm);
	BitBlt(hdc, i*tileWidth, j*tileWidth, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
}

void DrawBoard(HDC hdc)
{
	BITMAP bm;
	HDC hdcMem = CreateCompatibleDC(hdc);
	
	for(int i = 0; i < sizeX; i++){
		for(int j = 0; j < sizeY; j++){
			if(i%2==0){
				if(j%2==0){ DrawTile(hdc, hdcMem, i, j, bm, g_hbmLight); }
				else { DrawTile(hdc, hdcMem, i, j, bm, g_hbmDark); }	
			}
			else {
				if(j%2 != 0){ DrawTile(hdc, hdcMem, i, j, bm, g_hbmLight); }
				else { DrawTile(hdc, hdcMem, i, j, bm, g_hbmDark); }
			}
			
			if(flagArray[i*sizeY+j] == 1){
				DrawFlag(hdc, hdcMem, i, j);
			}					
		}
	}
	DeleteDC(hdcMem);
}

void DrawKnight(HDC hdc)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	
	SelectObject(hdcMem, g_hbmMask);
	BitBlt(hdc, g_knightInfo.x*tileWidth, g_knightInfo.y*tileWidth, g_knightInfo.width, g_knightInfo.height, hdcMem, 0, 0, SRCAND);

	SelectObject(hdcMem, g_hbmKnight);
	BitBlt(hdc, g_knightInfo.x*tileWidth, g_knightInfo.y*tileWidth, g_knightInfo.width, g_knightInfo.height, hdcMem, 0, 0, SRCPAINT);

	DeleteDC(hdcMem);
}

// check if the next move is possible
bool isMovePossible(chess_moves next_move) {
   	int i = next_move.x;
   	int j = next_move.y;
   	if ((i >= 0 && i < sizeX) && (j >= 0 && j < sizeY) && (flagArray[i*sizeY+j] == 0) ){
   		return true;
   	}
   	return false;
}

void UpdateKnight()
{
	if(drawKnight && totalMoves < sizeX*sizeY){
	chess_moves tempMove;
	chess_moves tempMoveTwo;
	int lowestMoveScore = 8;
	int tempScore = 0;
	chess_moves lowestScoringMove = {0,0};
	bool moveFound = false;
	
	chess_moves finalMove;
	chess_moves emergencyMove;
	
	bool finalMoveFound = false;
	bool emergencyMoveFound = false;
	
	//knight has been to this space
	if(((g_knightInfo.x/tileWidth) *sizeY+ (g_knightInfo.y/tileWidth)) < sizeX*sizeY){
		flagArray[(g_knightInfo.x) *sizeY+ (g_knightInfo.y)] = 1;
	}
	
	for(int i = 0; i < 8; i++){
		tempScore = 0;
		tempMove.x = g_knightInfo.x + moveArray[i].x;
		tempMove.y = g_knightInfo.y + moveArray[i].y;
		
		if(isMovePossible(tempMove)){
			for(int j = 0; j < 8; j++){
				tempMoveTwo.x = tempMove.x + moveArray[j].x;
				tempMoveTwo.y = tempMove.y + moveArray[j].y;
				if(isMovePossible(tempMoveTwo)){
					tempScore++;
				}
			}
			if(tempScore < lowestMoveScore && tempScore != 0){
				lowestMoveScore = tempScore;
				lowestScoringMove = tempMove;
				moveFound = true;	
			}
			else if(tempScore == lowestMoveScore && tempScore != 0){
				if(tempMove.x == 0 || tempMove.x == sizeX-1){
					lowestMoveScore = tempScore;
					lowestScoringMove = tempMove;
					moveFound = true;
				}
				else if(tempMove.y == 0 || tempMove.y == sizeY-1){
					lowestMoveScore = tempScore;
					lowestScoringMove = tempMove;
					moveFound = true;
				}
			}	
		}
	}
	
	if(!moveFound){
		for(int i = 0; i < 8; i++){
			tempMove.x = g_knightInfo.x + moveArray[i].x;
			tempMove.y = g_knightInfo.y + moveArray[i].y;
		
			if(isMovePossible(tempMove)){
				g_knightInfo.x = tempMove.x;
				g_knightInfo.y = tempMove.y;
				totalMoves++;
				return;
			}
		}
	}
	else{
		g_knightInfo.x = lowestScoringMove.x;
		g_knightInfo.y = lowestScoringMove.y;
		totalMoves++;
		return;
	}
}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	switch(Message) 
	{
		case WM_CREATE:
			{
				UINT ret;
				BITMAP bm;
			
				g_hbmKnight = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SPRITE));
				if(g_hbmKnight == NULL)
					MessageBox(hwnd, "Could not load IDB_KNIGT!", "Error", MB_OK | MB_ICONEXCLAMATION);

				g_hbmMask = CreateBitmapMask(g_hbmKnight, RGB(0, 0, 0));
				if(g_hbmMask == NULL)
					MessageBox(hwnd, "Could not create mask!", "Error", MB_OK | MB_ICONEXCLAMATION);
				
				g_hbmLight = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LIGHT_TILE));
				if(g_hbmLight == NULL)
					MessageBox(hwnd, "Could not load IDB_LIGHT_TILE!", "Error", MB_OK | MB_ICONEXCLAMATION);
				
				g_hbmDark = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DARK_TILE));
				if(g_hbmDark == NULL)
					MessageBox(hwnd, "Could not load IDB_DARK_TILE!", "Error", MB_OK | MB_ICONEXCLAMATION);
				
				g_hbmFlag = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_FLAG_SPRITE));
				if(g_hbmFlag == NULL)
				MessageBox(hwnd, "Could not load IDB_FLAG_SPRITE!", "Error", MB_OK | MB_ICONEXCLAMATION);
				
				g_hbmFlagMask = CreateBitmapMask(g_hbmFlag, RGB(0, 0, 0));
				if(g_hbmFlagMask == NULL)
					MessageBox(hwnd, "Could not create flag mask!", "Error", MB_OK | MB_ICONEXCLAMATION);
				
				GetObject(g_hbmKnight, sizeof(bm), &bm);

				ZeroMemory(&g_knightInfo, sizeof(g_knightInfo));
				g_knightInfo.width = bm.bmWidth;
				g_knightInfo.height = bm.bmHeight;
				
				g_knightInfo.x = knightX;
				g_knightInfo.y = knightY;

				ret = SetTimer(hwnd, ID_TIMER, ups, NULL);
				if(ret == 0)
					MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				DrawBoard(hdc);
				DrawKnight(hdc);

				EndPaint(hwnd, &ps);
			}
			break;
		case WM_TIMER:
			{
				HDC hdc = GetDC(hwnd);

				UpdateKnight();
				
				DrawBoard(hdc);
				DrawKnight(hdc);
			
				ReleaseDC(hwnd, hdc);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case CM_START:
					if(!tourStarted){
						tourStarted = true;
						drawKnight = true;
						//knightTour();
					}
					break;
				case CM_STARTPOINT:
					MessageBox (NULL, "Enter new start point: ", "Start point editor",0 );
					break;
				case CM_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case CM_ABOUT:
					MessageBox (NULL, "Knight's Tour simulation\nCreated using the Win32 API in Dev-C++\nMade by Christian Stiehl" , "About...", 0);
					break;
				case CM_GRID_5:
					Resize(5, hwnd);
					break;
				case CM_GRID_6:
					Resize(6, hwnd);
					break;
				case CM_GRID_7:
					Resize(7, hwnd);
					break;
				case CM_GRID_8:
					Resize(8, hwnd);
					break;
				case CM_GRID_9:
					Resize(9, hwnd);
					break;
				case CM_GRID_10:
					Resize(10, hwnd);
					break;
			}
			break;
		case WM_DESTROY:
			KillTimer(hwnd, ID_TIMER);
			
			DeleteObject(g_hbmKnight);
			DeleteObject(g_hbmMask);
			DeleteObject(g_hbmLight);
			DeleteObject(g_hbmDark);
			
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	MSG Msg;
	HWND hwnd;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
	wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = "MAINMENU";
	wc.lpszClassName = "WindowClass";

	if(!RegisterClassEx(&wc)) {
		MessageBox(0,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Knight's Tour",WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		10,
		10,
		(sizeX*tileWidth)+10,(sizeY*tileWidth)+52,
		NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
		MessageBox(0, "Window Creation Failed!", "Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);
		return 0;
	}

	ShowWindow(hwnd,1);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
