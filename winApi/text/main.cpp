#pragma once
#include <windows.h>
#include "textWindow.h"
#include "editCtrlWindow.h"
#include <cstdlib>
#include <ctime>
#include "resource.h"
#include <iostream>

LPTSTR loadInitText()
{
	HRSRC hres = FindResource( NULL, MAKEINTRESOURCE( CUSTOM_STRING ), MAKEINTRESOURCE( RT_MYDATA ) );
	if( hres == 0 ) { 
		std::cerr << "Error of loading resource";
	}
	HGLOBAL hbytes = LoadResource( NULL, hres );
    LPVOID pdata = LockResource( hbytes ); // для чего? просто чтобы было другим недоступно?
	LPTSTR sData = reinterpret_cast<LPTSTR>( reinterpret_cast<LPBYTE>( pdata ) );
	return sData;
}

int __stdcall wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
               PWSTR szCmdLine, int CmdShow )
{
    
	srand( static_cast<size_t>( time( NULL ) ) );
	TextWindow::RegisterClass( L"MainWindowClass", hInstance );
	TextWindow parent_window( loadInitText() );

	if( !parent_window.Create( L"MainWindowClass", hInstance ) ) { 
		MessageBox( 0, L"Error", L"Error", MB_OK );
		return -1;
	} 
	parent_window.Show( CmdShow );
	
	MSG msg;
	HACCEL hAccelerators = LoadAcceleratorsW( hInstance, MAKEINTRESOURCEW( IDR_ACCELERATOR ) );
	while( GetMessage( &msg, NULL, 0, 0 ) > 0 ) {
		if( !TranslateAcceleratorW( msg.hwnd, hAccelerators, &msg ) ||
			!IsDialogMessage( parent_window.GetEditHandle(), &msg ) ) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	
	return 0;
}