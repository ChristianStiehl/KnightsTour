#include <windows.h>
#include "resource.h"
#include <time.h>
#include <sstream>

using namespace std;

//bitmap variable declaration
HBITMAP g_hbmKnight = NULL;
HBITMAP g_hbmMask = NULL;
HBITMAP	g_hbmLight = NULL;
HBITMAP g_hbmDark = NULL;
HBITMAP g_hbmFlag = NULL;
HBITMAP g_hbmFlagMask = NULL;

int sizeX = 8; //width of the grid
int sizeY = 8; //height of the grid
int tileWidth = 75; //widht of each square

bool tourStarted = false; //bool to check if a tour is in progress
bool updateKnight = false; //bool to check if the knight should start calculating a route

int *flagArray = new int[sizeX*sizeY]; //array to check what tiles have been visited
int totalMoves = 0; //number of moves taken

int ups = 500; //update duration in milliseconds
const int ID_TIMER = 1; //timer declaration

HWND hwnd;

//defines a structure for the knight
typedef struct _KNIGHTINFO
{
	//width and height of the sprite
	int width, height;
	//x and y position on the board
	int x, y;
}KNIGHTINFO;

//defines a structure for a chess move
typedef struct chess_moves {
   //x and y position on the board
   int x,y;
}chess_moves;

KNIGHTINFO g_knightInfo;
//create moveArray and fill it with all 8 possible moves a knight could make from any square
chess_moves moveArray[8] = { {2,1}, {1,2},{-1,2},{-2,1}, {-2,-1},{-1,-2},{1,-2},{2,-1} };

//function to resize the board, update the flag array size and resize the window
void Resize(int newX, int newY, HWND hwnd)
{
	if(!tourStarted){
		sizeX = newX;
		sizeY = newY;
		g_knightInfo.x = 0;
		g_knightInfo.y = 0;
		flagArray = new int[sizeX*sizeY];
		for(int i = 0; i < sizeX*sizeY; i++){
			flagArray[i] = 0;
		}
		SetWindowPos(hwnd, NULL, 10, 10, (sizeX*tileWidth)+10, (sizeY*tileWidth)+52, SWP_SHOWWINDOW);
	}	
}

//function to create bitmap masks for transparency
//source: forgers win32 tutorial, see notes
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

//function to draw a flag
void DrawFlag(HDC hdc, HDC hdcMem, int i, int j)
{
	RECT rec;
	SetRect(&rec, i*75, j*75, i*75+75, j*75+75);
	
	SelectObject(hdcMem, g_hbmFlagMask);
	BitBlt(hdc, i*tileWidth, j*tileWidth, tileWidth, tileWidth, hdcMem, 0, 0, SRCAND);

	SelectObject(hdcMem, g_hbmFlag);
	BitBlt(hdc, i*tileWidth, j*tileWidth, tileWidth, tileWidth, hdcMem, 0, 0, SRCPAINT);
	
	std::stringstream buffer;
    buffer << flagArray[i*sizeY+j];

    DrawText(hdc, buffer.str().c_str(), -1, &rec, DT_SINGLELINE);
}

//function to draw a tile
void DrawTile(HDC hdc, HDC hdcMem, int i,int j, BITMAP bm, HBITMAP hbm)
{
	SelectObject(hdcMem, hbm);
	GetObject(g_hbmLight, sizeof(bm), &bm);
	BitBlt(hdc, i*tileWidth, j*tileWidth, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
}

//function that loops trough the entire board and calls DrawTile and DrawFlag when needed
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
			
			if(flagArray[i*sizeY+j] >= 1){
				DrawFlag(hdc, hdcMem, i, j);
			}					
		}
	}
	DeleteDC(hdcMem);
}

//function that draws the knight on his position
void DrawKnight(HDC hdc)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	
	SelectObject(hdcMem, g_hbmMask);
	BitBlt(hdc, g_knightInfo.x*tileWidth, g_knightInfo.y*tileWidth, g_knightInfo.width, g_knightInfo.height, hdcMem, 0, 0, SRCAND);

	SelectObject(hdcMem, g_hbmKnight);
	BitBlt(hdc, g_knightInfo.x*tileWidth, g_knightInfo.y*tileWidth, g_knightInfo.width, g_knightInfo.height, hdcMem, 0, 0, SRCPAINT);

	DeleteDC(hdcMem);
}

//function to check if a move would place the knight out of bounds or on a square that has already been visited
bool isMovePossible(chess_moves next_move) {
   	int i = next_move.x;
   	int j = next_move.y;
   	if ((i >= 0 && i < sizeX) && (j >= 0 && j < sizeY) && (flagArray[i*sizeY+j] == 0) ){
   		return true;
   	}
   	return false;
}

//function to calculate the next move of the knight, gets called by WM_TIMER 
void UpdateKnight()
{
	if(updateKnight && totalMoves < sizeX*sizeY){ //only need to calculate if the knight should move and the tour hasn't been completed yet
		chess_moves tempMove, tempMoveTwo, lowestScoringMove; //empty chess moves to store possible moves
		int lowestMoveScore = 8;
		int tempScore;
		bool moveFound = false;
	
		//mark the current tile as visited in the flagArray
		if(((g_knightInfo.x/tileWidth) *sizeY+ (g_knightInfo.y/tileWidth)) < sizeX*sizeY){
			flagArray[(g_knightInfo.x) *sizeY+ (g_knightInfo.y)] = totalMoves+1;
		}
		
		//these for loops go to 8 because there are 8 moves possible in moveArray, does not get affected by board size
		for(int i = 0; i < 8; i++){
			//reset score to 0 and apply the next possible move to the current position
			tempScore = 0;
			tempMove.x = g_knightInfo.x + moveArray[i].x;
			tempMove.y = g_knightInfo.y + moveArray[i].y;
		
			//if the new move is possible calculate its value, else try the next possible move
			if(isMovePossible(tempMove)){
				//for each move that is possible, check how many moves are possible from that tile, result is it's score
				for(int j = 0; j < 8; j++){
					tempMoveTwo.x = tempMove.x + moveArray[j].x;
					tempMoveTwo.y = tempMove.y + moveArray[j].y;
					if(isMovePossible(tempMoveTwo)){
						//increase score
						tempScore++;
					}
				}
				
				//after calculating a moves score check if its lower than the current lowest score but not 0
				if(tempScore < lowestMoveScore && tempScore != 0){
					lowestMoveScore = tempScore;
					lowestScoringMove = tempMove;
					moveFound = true;	
				}
				//if its not lower, check if its equal to the current lowest score but not 0
				else if(tempScore == lowestMoveScore && tempScore != 0){
					//if its equal check if the new move will place the knight along the edge of the board, these moves are prefered
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
	
		//if no move that did not have a score of 9 was found, do any move that is possible (usually the last move)
		if(!moveFound){
			for(int i = 0; i < 8; i++){
				tempMove.x = g_knightInfo.x + moveArray[i].x;
				tempMove.y = g_knightInfo.y + moveArray[i].y;
			
				if(isMovePossible(tempMove)){
					g_knightInfo.x = tempMove.x;
					g_knightInfo.y = tempMove.y;
				}
			}
		}
		//if a move was found, update the knights position accordingly
		else{
			g_knightInfo.x = lowestScoringMove.x;
			g_knightInfo.y = lowestScoringMove.y;
		}
		//increase the amount of moves taken
		totalMoves++;
	}
}

//win 32 callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	switch(Message) 
	{
		//on create load all bitmaps, set knight position, fill flagArray with 0 and start the timer
		case WM_CREATE:
			{
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
				
				g_knightInfo.x = 0;
				g_knightInfo.y = 0;
				
				for(int i = 0; i < sizeX*sizeY; i++){
					flagArray[i] = 0;
				}

				UINT ret = SetTimer(hwnd, ID_TIMER, ups, NULL);
				if(ret == 0)
					MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		//WM_PAINT is only called when the window is resized and on the initial creation
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				DrawBoard(hdc);
				DrawKnight(hdc);

				EndPaint(hwnd, &ps);
			}
			break;
		//WM_TIMER is the 'gameloop' of this program, calls update and draw functions every 500 ms
		case WM_TIMER:
			{
				HDC hdc = GetDC(hwnd);

				UpdateKnight();
				
				DrawBoard(hdc);
				DrawKnight(hdc);
			
				ReleaseDC(hwnd, hdc);
			}
			break;
		//Menu commands that are defined in resource.h and created in resource.rc
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				//start the tour
				case CM_START:
					if(!tourStarted){
						tourStarted = true;
						updateKnight = true;
					}
					break;
				//pause the tour
				case CM_STOP:
					tourStarted = false;
					updateKnight = false;
					break;
				//reset the tour and place the knight back to 0,0
				case CM_RESET:
					tourStarted = false;
					updateKnight = false;
					totalMoves = 0;
					Resize(sizeX, sizeY, hwnd);
					break;
				//place the knight on a random position (time(NULL) is used as seed for rand
				case CM_RANDOM:
					if(!tourStarted){
						srand(time(NULL));
						g_knightInfo.x = rand() %sizeX;
						g_knightInfo.y = rand() %sizeY;
					}
					break;
				//place the knight in a centered tile, because g_knightInfo.x and y are both ints, these are automatically rounded towards zero
				case CM_CENTER:
					if(!tourStarted){
						g_knightInfo.x = sizeX/2;
						g_knightInfo.y = sizeY/2;
					}
					break;
				//place the knight in the left corner: 0,0
				case CM_LEFTCORNER:
					if(!tourStarted){
						g_knightInfo.x = 0;
						g_knightInfo.y = 0;
					}
					break;
				//change timer to tick once every second (1000 ms)
				case CM_SLOW:
					ups = 1000;
					SetTimer(hwnd, ID_TIMER, ups, NULL);
					break;
				//change timer to tick twice per second (500ms)
				case CM_NORMAL:
					ups = 500;
					SetTimer(hwnd, ID_TIMER, ups, NULL);
					break;
				//change timer to tick four times per second (250ms)
				case CM_FAST:
					ups = 250;
					SetTimer(hwnd, ID_TIMER, ups, NULL);
					break;
				case CM_EXTREME:
					ups = 50;
					SetTimer(hwnd, ID_TIMER, ups, NULL);
					break;
				case CM_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case CM_ABOUT:
					MessageBox (NULL, "Knight's Tour simulation\nCreated using the Win32 API in Dev-C++\nMade by Christian Stiehl" , "About...", 0);
					break;
				//all CM_GRID commands resize the grid
				case CM_GRID_5:
					Resize(5, 5, hwnd);
					break;
				case CM_GRID_6:
					Resize(6, 6, hwnd);
					break;
				case CM_GRID_7:
					Resize(7, 7, hwnd);
					break;
				case CM_GRID_8:
					Resize(8,8, hwnd);
					break;
				case CM_GRID_9:
					Resize(9,9, hwnd);
					break;
				case CM_GRID_10:
					Resize(10,10,  hwnd);
					break;
				case CM_GRID_15:
					Resize(15,15, hwnd);
					break;
				case CM_GRID_RECT:
					Resize(16,8, hwnd);
					break;
			}
			break;
		case WM_DESTROY:
			KillTimer(hwnd, ID_TIMER);
			
			DeleteObject(g_hbmKnight);
			DeleteObject(g_hbmMask);
			DeleteObject(g_hbmLight);
			DeleteObject(g_hbmDark);
			DeleteObject(g_hbmFlag);
			DeleteObject(g_hbmFlagMask);
			DeleteObject(flagArray);
			
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

//win 32 window creation, default code,  
//only changed initial size determination, name and included WS_THICKFRAME in the overlapped window so manual resizing is disabled
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
