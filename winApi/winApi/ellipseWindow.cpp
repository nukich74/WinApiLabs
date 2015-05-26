#include "ellipseWindow.h"

EllipseWindow::EllipseWindow( std::pair< int, int > sp, std::pair< int, int > l, int r, 
		Color bgColor, Color figColor ) : 
	speed(sp), center(l), radius(r), curBgColor(bgColor), curFigColor(figColor),
	selectBgColor( Color( 0, 168, 107 ) ), selectFigColor( Color( 127, 199, 255 ) )
{
}

EllipseWindow::~EllipseWindow()
{
}

bool EllipseWindow::RegisterClass( std::wstring className, HINSTANCE hinstance )
{
	WNDCLASSEX wc;	
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = EllipseWindow::windowProc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursor( NULL, IDC_WAIT );
    wc.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName = NULL;
	wc.lpszClassName = className.c_str();
    wc.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
	ATOM res = RegisterClassEx( &wc );
	return res != 0;
}

LRESULT __stdcall EllipseWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam ) {
	if ( message == WM_NCCREATE ) {
		SetWindowLong( handle, GWL_USERDATA, reinterpret_cast<LONG>((reinterpret_cast<CREATESTRUCT*>(lParam))->lpCreateParams) );
	}
	EllipseWindow* wnd = reinterpret_cast< EllipseWindow* >( GetWindowLong(handle, GWL_USERDATA) );
	switch (message) {
		case WM_CREATE:
			wnd->timer = SetTimer( handle, 0, 1, NULL );
			return 1;
		case WM_TIMER:
			wnd->moveEllipse();
			wnd->invalidate();
			return 0;
		case WM_DESTROY:
			KillTimer( handle, wnd->timer );
			wnd->onDestroy();
			break;
		case WM_PAINT:
			wnd->drawEllipse();
			return 0;
		case WM_LBUTTONDOWN:
			if( ::SetFocus( handle ) == 0 ) {
				MessageBox( 0, L"Error", L"Error", MB_OK );
			}
			return 0;
	}
	return DefWindowProc ( handle, message, wParam, lParam );
}

void EllipseWindow::invalidate() {
	RECT rect;
	if (GetClientRect(handle, &rect)) {
		InvalidateRect(handle, &rect, false);
	}
}

void EllipseWindow::drawEllipse() {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint( handle, &ps );
	if( hdc == 0 ) {
		return;
	}
	HDC newHdc = CreateCompatibleDC(hdc);
	RECT client_rect;
	GetClientRect(handle, &client_rect);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);
	ps.rcPaint = client_rect;
	SelectObject(newHdc, bitmap);
	if( newHdc == 0 ) {
		return;
	}
	HBRUSH backGroundBrush;
	HBRUSH ellipseBrush;
	HGDIOBJ oldBrush;
	if( handle == ::GetFocus() ) {
		backGroundBrush = CreateSolidBrush( RGB( selectBgColor.red, selectBgColor.green, selectBgColor.blue ) ); //(155, 252, 155)
		ellipseBrush = CreateSolidBrush( RGB( selectFigColor.red, selectFigColor.green, selectFigColor.blue ) ); //(255, 136, 0)
	} else {
		backGroundBrush = CreateSolidBrush( RGB( curBgColor.red, curBgColor.green, curBgColor.blue ) ); //(155, 252, 155)
		ellipseBrush = CreateSolidBrush( RGB( curFigColor.red, curFigColor.green, curFigColor.blue ) ); //(255, 136, 0)
	}
	if( backGroundBrush && ellipseBrush ) { 
		oldBrush = (HGDIOBJ) SelectObject( newHdc, backGroundBrush );
		FillRect( newHdc, &ps.rcPaint, backGroundBrush );
	
		SelectObject( newHdc, ellipseBrush );
		Ellipse( newHdc, center.first - radius, center.second - radius, center.first + radius, center.second + radius );
	
		BitBlt( hdc, client_rect.left, client_rect.top, client_rect.right, client_rect.bottom, 
		     newHdc, client_rect.left, client_rect.top, SRCCOPY );
		SelectObject( newHdc, oldBrush );
		DeleteObject( bitmap );
		DeleteDC( newHdc);
		EndPaint( handle, &ps );
		DeleteObject( backGroundBrush );
		DeleteObject( ellipseBrush );
			( oldBrush );
	}
}

void EllipseWindow::moveEllipse() {
	std::pair< int, int > newCord(center.first + speed.first, center.second + speed.second);
	std::swap(newCord, center);
	RECT border;
	GetClientRect(handle, &border);
	if( speed.first < 0 ) {
		if( center.first - radius <= border.left ) {
			speed.first = -speed.first;
			center.first = border.left + radius;
		}
	} else {
		if( center.first + radius >= border.right ) {
			speed.first = -speed.first;
			center.first = border.right - radius;
		}
	}
	if( speed.second < 0 ) {
		if( center.second - radius <= border.top ) {
			speed.second = -speed.second;
			center.second = border.top + radius;
		}
	} else {
		if( center.second + radius >= border.bottom ) {
			speed.second = -speed.second;
			center.second = border.bottom - radius;
		}
	}
}


bool EllipseWindow::Create( std::wstring className, HINSTANCE hInstance ) {
	handle = CreateWindowEx(
		NULL,
		className.c_str(),
        L"EllipseWindow",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, this );
	return ( handle != NULL );
}

bool EllipseWindow::Create( std::wstring className, HINSTANCE hInstance, HWND pHandle, int index ) {
	handle = CreateWindowEx(
		NULL,
		className.c_str(),
        L"EllipseWindow",
        WS_CHILDWINDOW | WS_BORDER | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        pHandle, (HMENU) (int) ( 997 + index ), hInstance, this );
	return ( handle != NULL );
}

void EllipseWindow::Show( int cmdShow ) {
	ShowWindow( handle, cmdShow );
	//UpdateWindow( handle );
}

void EllipseWindow::onDestroy() {
	PostQuitMessage( 0 );
}


HWND EllipseWindow::GetHandle() {
	return handle;
}

void EllipseWindow::SetRadius( int r ) {
	this->radius = r;
}

int EllipseWindow::GetRadius() {
	return this->radius;
}