#pragma once
#include <Windows.h>
#include <Commctrl.h>
#include "resource.h"
#include "editCtrlWindow.h"
#include <string>
#include <vector>
#include "SettingsKeeper.h"

class TextWindow {
public: 
	TextWindow();
	TextWindow( LPTSTR );
	~TextWindow();
	
	static bool RegisterClass( std::wstring, HINSTANCE );
	bool Create( std::wstring, HINSTANCE );	
	void Show( int cmdShow );

	wchar_t* GetEditCtrlWndText( int& ) const;
	HWND GetHandle() const;
	HWND GetEditHandle() const;
	HWND GetSettingHandle() const;
	bool HasChangedTextInBox() const;
	
	void SetSettingsHandle( HWND handle ) { this->settingsHandle = handle; }
	void SetEditHadle( HWND handle ) { this->editHandle = handle; }
	void SetHandle( HWND handle ) {	this->handle = handle; }
	void SetIfTextInBox( bool val ) { this->isTextInBox = val; }
	static BOOL __stdcall dlgProc( HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

protected:
	void onDestroy();

private:
	SettingsKeeper mainSettings;
	SettingsKeeper previewSettings;
	LPTSTR	initText;
	wchar_t* wndTitle;
	bool isTextInBox;
	HWND handle;
	HWND editHandle;
	HWND settingsHandle;
	EditCtrlWindow* editWnd;

	static LRESULT __stdcall windowProc( HWND, UINT, WPARAM, LPARAM );
	void onCreate( RECT rect );
	void onResize();
	void onClose();
	void onSettingsInit( HWND );
	void onSettingsCommand();
	void onSettingsHScroll( HWND );
	bool onSave();
	void onSaveNewSettings();
	void onRestoreOldSettings();
	void onSettingsFontColor( HWND );
	void onSettingsBgColor( HWND );
	void onSettingsPreview( HWND );
	LRESULT onOnCTLColorEdit( WPARAM, LPARAM );
	void settingsDbx();
	void saveToFile( LPCTSTR );
	void writeToFile( HANDLE, wchar_t*, int );
	void updateSettings();
	void reDrawTextEditControl();
	CHOOSECOLOR initChooseColor(HWND ownerHandle);
};