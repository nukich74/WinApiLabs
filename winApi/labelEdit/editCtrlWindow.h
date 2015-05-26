#pragma once
#include <windows.h>
#include <string>

class EditCtrlWindow {
public:
	EditCtrlWindow();
	~EditCtrlWindow();

	bool Create( HINSTANCE, HWND, int );	
	void Show( int cmdShow );

	HWND GetHandle() const;
	void SetHandle( HWND );
	
protected:

private:
	HWND handle;
	std::wstring text;
};