#include <windows.h>
#include "TestWindow.h"
#include "ellipseWindow.h"
#include <cstdlib>
#include <ctime>

int __stdcall wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
               PWSTR szCmdLine, int CmdShow )
{
	srand( static_cast<size_t>( time( NULL ) ) );
	TestWindow::RegisterClass( L"parentWindowClass", hInstance );
	EllipseWindow::RegisterClass( L"ellipseClass", hInstance);
	TestWindow parent_window( 105, 15 );
	if( !parent_window.Create( L"parentWindowClass", hInstance ) ) { 
		MessageBox( 0, L"Error", L"Error", MB_OK );
		return -1;
	} 
	parent_window.Show( CmdShow );

	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) > 0 ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	
	return 0;
}