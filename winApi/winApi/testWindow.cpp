#include "testWindow.h"
#include <algorithm>
#include <Windows.h>

TestWindow::TestWindow() { }
TestWindow::~TestWindow() { } 

bool TestWindow::RegisterClass( std::wstring className, HINSTANCE hInstance ) {
	WNDCLASSEX wc;	
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TestWindow::windowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor( NULL, IDC_WAIT );
    wc.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName = NULL;
	wc.lpszClassName = className.c_str();
    wc.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
	ATOM res = RegisterClassEx( &wc );
	return ( res != 0 );
}

LRESULT __stdcall TestWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam ) {
	if( message == WM_NCCREATE ) {
		SetWindowLong( handle, GWL_USERDATA, reinterpret_cast<LONG>( ( reinterpret_cast<CREATESTRUCT*>( lParam ) )->lpCreateParams ) );
		TestWindow* wnd = reinterpret_cast<TestWindow*>( ( reinterpret_cast<CREATESTRUCT*>( lParam ) )->lpCreateParams );
			//reinterpret_cast< TestWindow* >( GetWindowLong( handle, GWL_USERDATA) );
		wnd->SetHandle( handle );
	}

	TestWindow* wnd = reinterpret_cast< TestWindow* >( GetWindowLong( handle, GWL_USERDATA) );
	if( message == WM_CREATE) {
		RECT rect;
		GetClientRect( handle, &rect );///onCreate() 
		wnd->onCreate( rect );	
	}
	switch (message) {
		case WM_DESTROY:
			wnd->onDestroy();
			return 0;
		case WM_SIZE:///
			wnd->onResize(); 
			return 0;
	}
	return DefWindowProc ( handle, message, wParam, lParam );
}

BOOL TestWindow::ProcChild( HWND handle, LPARAM lParam ) {
	std::pair< RECT, std::pair< int, int > >* data = (std::pair< RECT, std::pair< int, int > >*) lParam;
	int childNum = (GetWindowLong( handle, GWL_ID)) % 997;
	int extraLine = 0;
	int extraHeight = 0;
	int extraWidth = 0;
	if( data->second.second % data->second.first != 0 ) {
		extraLine++;
	}
	if( ( childNum + 1 ) % data->second.first == 0 ) {
		extraWidth += ( data->first.right % data->second.first );
	}
	if( ( childNum ) / data->second.first + 1 == data->second.second / data->second.first + extraLine) {
		extraHeight += ( data->first.bottom % ( data->second.second / data->second.first + extraLine ) );
	}
	int x = ( data->first.right / data->second.first ) * ( childNum % data->second.first );
	int y = ( childNum / data->second.first ) * ( data->first.bottom / ( ( data->second.second / data->second.first ) + extraLine ) );
	int w = ( data->first.right / data->second.first );
	int h = ( data->first.bottom / ( data->second.second / data->second.first + extraLine ) ); 
	if( data != NULL ) {
		MoveWindow( handle, x, y, w + extraWidth, h + extraHeight, TRUE); 
	}
	EllipseWindow* wnd = reinterpret_cast<EllipseWindow*>( GetWindowLong( handle, GWL_USERDATA ) );
	if( wnd->GetRadius() * 2 >= min( w, h ) ) {
		wnd->SetRadius( wnd->GetRadius() / 6 );
	}
	wnd->Show( SW_SHOW );
	return TRUE;
}

bool TestWindow::Create( std::wstring className, HINSTANCE hInstance) {
	handle = CreateWindowEx(
		NULL,
		className.c_str(),
        L"TestWindow",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, this );
	return handle != NULL;
}

void TestWindow::Show( int cmdShow ) {
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

void TestWindow::onDestroy() {
	PostQuitMessage( 0 );
}


HWND TestWindow::GetHandle() {
	return handle;
}

int TestWindow::getChildPerLineSize()  {
	return this->childPerLineSize;
}

int TestWindow::getChildWndCount() {
	return this->childWndCount;
}

void TestWindow::onCreate( const RECT& rect ) {
	for( int i = 0; i < this->getChildWndCount(); i++ ) {
		EllipseWindow* window = new EllipseWindow( std::make_pair( rand() % 5 + 1, rand() % 5 + 1 ), std::make_pair( rand() % rect.right, rand() % rect.bottom ), 
			rand() % 20 + 5, Color(155, 252, 155), Color(255, 136, 0) );
		if( !window->Create( L"ellipseClass", GetModuleHandle(NULL), this->handle, i ) ) {
			MessageBox( 0, L"Error", L"Error", MB_OK );
		}
		this->childList.push_back(window);
	}
}

void TestWindow::onResize() {
	std::pair< RECT, std::pair< int, int > > data;
	data.second.first = this->getChildPerLineSize();
	data.second.second = this->getChildWndCount();
	GetClientRect( handle, &data.first ); 
	EnumChildWindows( handle, TestWindow::ProcChild, (LPARAM)&data );
}