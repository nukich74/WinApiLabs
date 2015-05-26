#pragma once
#include <Windows.h>
#include "ellipseWindow.h"
#include <string>
#include <vector>

class TestWindow {
public: 
	TestWindow();
	TestWindow( int childWndCount, int childPerLineSize ) : childPerLineSize( childPerLineSize ), childWndCount( childWndCount ) { }
	
	~TestWindow();
	
	static bool RegisterClass( std::wstring, HINSTANCE );
	
	bool Create( std::wstring, HINSTANCE );
	
	void Show( int cmdShow );

	HWND GetHandle();

	int getChildPerLineSize();
	int getChildWndCount();
	void SetHandle( HWND handle ) {
		this->handle = handle;
	}

protected:
	void onDestroy();

private:
	void onCreate( const RECT& rect );
	void onResize();
	int childWndCount, childPerLineSize;
	std::vector< EllipseWindow* > childList;
	HWND handle;
	static BOOL __stdcall ProcChild( HWND , LPARAM );
	static LRESULT __stdcall windowProc( HWND, UINT, WPARAM, LPARAM );
};