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
int moveIndex = 0;
int ups = 500;
const int ID_TIMER = 1;

HWND hwnd;

int *tour = new int[sizeX*sizeY]; 

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

// check if the next move is possible
bool isMovePossible(chess_moves next_move) {
   int i = next_move.x;
   int j = next_move.y;
   if ((i >= 0 && i < sizeX) && (j >= 0 && j < sizeY) && (flagArray[i*sizeY+j] == 0) )
      return true;
   return false;
}

// recursive function to find a knight tour
bool findTour(chess_moves move_KT[], int x, int y, int move_count) {
   int i;
   chess_moves next_move = {0,0};
   
   if (move_count == sizeX*sizeY-1) {
      // Knight tour is completed i.e all cells on the
      // chess board has been visited by knight once 
      return true;
   }

   // try out the possible moves starting from the current coordinate
   for (i = 0; i < 8; i++) {
      // get the next move
      next_move.x = x + move_KT[i].x;
      next_move.y = y + move_KT[i].y;

      if (isMovePossible(next_move)) {
         // if the move is possible
         // increment the move count and store it in tour matrix
         tour[next_move.x * sizeY + next_move.y] = move_count+1;
         if (findTour(move_KT, next_move.x, next_move.y, move_count+1) == true) {
            return true;
         }
         else {
            // this move was invalid, try out other possiblities 
            tour[next_move.x * sizeY + next_move.y] = 0;
         }
      }
   }
   return false;
}

// wrapper function
void knightTour() {
   	int i,j;

   	// initialize tour matrix
   	for (i = 0; i < sizeX; i++) {
   	   for (j = 0; j < sizeY; j++) {
   	      tour[i * sizeY + j] = 0;
   	   }
   	}

   	// all possible moves that knight can take
   	chess_moves move_KT[8] = { {2,1}, {1,2},{-1,2},{-2,1}, {-2,-1},{-1,-2},{1,-2},{2,-1} };

   	// knight tour starting point
   	chess_moves curr_move = {g_knightInfo.x, g_knightInfo.y};

   	// find a possible knight tour using a recursive function
   	// starting from current move 
   	if(findTour(move_KT, knightX, knightY, 0) == false) {
     	 MessageBox(hwnd, "Knight's Tour does not exist from this position on this board!", "Error", MB_OK | MB_ICONEXCLAMATION);
     	 tourStarted = false;
   	}
   	else {
   	   //draw knight along tour
   		moveIndex = 0;
   	   drawKnight = true;
   	}
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

void UpdateKnight()
{
	if(drawKnight){
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
			if(tempScore < lowestMoveScore){
				if(tempScore = 0){
					finalMove = tempMove;
					finalMoveFound = true;
				}
				else if(tempScore = 1){
					emergencyMove = tempMove;
					emergencyMoveFound = true;
				}
				else {
					lowestMoveScore = tempScore;
					lowestScoringMove = tempMove;
					moveFound = true;	
				}
			}	
		}
	}
	
	if(moveFound){
		g_knightInfo.x = lowestScoringMove.x;
		g_knightInfo.y = lowestScoringMove.y;
	}
	else if(emergencyMoveFound){
		g_knightInfo.x = emergencyMove.x;
		g_knightInfo.y = emergencyMove.y;
	}
	else if(finalMoveFound){
		g_knightInfo.x = finalMove.x;
		g_knightInfo.y = finalMove.y;
	}
}

	/*
	if(drawKnight){
		for(int i= 0; i < sizeX; i++){
			for (int j = 0; j < sizeY; j++) {
				if(tour[i * sizeY +j] == moveIndex){
					g_knightInfo.x = i;
					g_knightInfo.y = j;
				
					if(((g_knightInfo.x/tileWidth) *sizeY+ (g_knightInfo.y/tileWidth)) < sizeX*sizeY){
						flagArray[(g_knightInfo.x) *sizeY+ (g_knightInfo.y)] = 1;
					}
					moveIndex++;
					return;
				}
      		}
		}
	}
	*/
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
