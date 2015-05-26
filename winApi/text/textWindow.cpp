#include "textWindow.h"
#include "editCtrlWindow.h"
#include <algorithm>
#include <iostream>
#include <counter.h>
#include <sstream>

TextWindow::TextWindow() :
	isTextInBox( false ), mainSettings(), previewSettings()
{
	this->editHandle = NULL;
	this->handle = NULL;
	this->settingsHandle = NULL;
}

TextWindow::TextWindow( LPTSTR ptr ) : 
	isTextInBox( false ), mainSettings(), previewSettings( mainSettings )

{
	handle = NULL;
	editHandle = NULL;
	settingsHandle = NULL;
	this->initText = ptr;
}


TextWindow::~TextWindow()
{	
	delete this->wndTitle;
} 

bool TextWindow::RegisterClass( std::wstring className, HINSTANCE hInstance ) 
{
	WNDCLASSEX wc;	
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TextWindow::windowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
	wc.lpszMenuName = MAKEINTRESOURCE( IDR_MYMENU );
	wc.lpszClassName = className.c_str();
	wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wc.hIconSm = ( HICON )LoadImage( hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 16, 16, 0 );
	ATOM res = RegisterClassEx( &wc );
	return ( res != 0 );
}

LRESULT __stdcall TextWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam ) 
{
	if( message == WM_NCCREATE ) {
		SetWindowLong( handle, GWL_USERDATA, reinterpret_cast<LONG>( ( reinterpret_cast<CREATESTRUCT*>( lParam ) )->lpCreateParams ) );
		TextWindow* wnd = reinterpret_cast< TextWindow* >( GetWindowLong( handle, GWL_USERDATA) );
		wnd->SetHandle( handle );
	}
	
	TextWindow* wnd = reinterpret_cast< TextWindow* >( GetWindowLong( handle, GWL_USERDATA) );
	if( message == WM_CREATE ) {
		RECT rect;
		GetClientRect( handle, &rect ); 
		wnd->SetHandle( handle );
		wnd->onCreate( rect );	
	}
	switch( message ) {
		case WM_CLOSE: 
			wnd->onClose();
			return 0;
		case WM_DESTROY:
			wnd->onDestroy();
			return 0;
		case WM_SIZE:
			wnd->onResize(); 
			return 0;
		case WM_CTLCOLOREDIT:
			return wnd->onOnCTLColorEdit( wParam, lParam );
		case WM_COMMAND: 
			if( HIWORD( wParam ) == 0 ) { //Menu
				switch( LOWORD( wParam ) ) {
					case ID_FILE_EXIT:
						wnd->onClose();
						return 0;
					case ID_FILE_SAVE:  //save to file
						if( !wnd->onSave() ) {
							MessageBox( wnd->GetHandle(), L"File wasn't saved", L"Save", MB_OK );
						}
					case ID_DBX_SET: //settings dialog box
						wnd->settingsDbx();
						return 0;
					case ID_CNT_WRD:
						int len;
						wchar_t* tStr = wnd->GetEditCtrlWndText( len );
						int wordCount = WordUtils::WordCounter( tStr ); 
						std::wstringstream wss;
						wss << wordCount;
						MessageBox( wnd->GetHandle(), wss.str().c_str(), L"Word count", MB_OK );
						
						HINSTANCE hModule = ::LoadLibrary(  L"WordUtilsLoad.dll" );
						
						if( hModule != NULL ) {
							int ( *MY_FUNC ) ( const WCHAR* );
							reinterpret_cast<FARPROC&>( MY_FUNC ) = ::GetProcAddress( hModule, "WordCounterHex" );
							if( MY_FUNC == NULL ) {
								MessageBox( wnd->GetHandle(), L"Error of loading function from WordCountLoad.dll", L"WC ERROR", MB_OK );
							} else {
								auto hexValue = MY_FUNC( tStr );
								std::wstringstream wss2;
								wss2 << hexValue;
								MessageBox( wnd->GetHandle(), wss2.str().c_str(), L"Word count", MB_OK );
							}
							::FreeLibrary( hModule );
						} else {
							MessageBox( wnd->GetHandle(), L"Error of loading WordCountLoad.dll", L"WC ERROR", MB_OK );
						}
						return 0;
				}
			} else if( HIWORD( wParam ) == 1 ) { //Acceler
				switch( LOWORD( wParam ) ) {
					case ID_FAST_EXIT:
						::DestroyWindow( wnd->GetHandle() );
						return 0;
				}
			} else { //control
				switch( HIWORD( wParam ) ) {
					case EN_CHANGE:
						wnd->SetIfTextInBox( true );
						return 0;
				}
			}
	}
	return DefWindowProc ( handle, message, wParam, lParam );
}

void TextWindow::settingsDbx() 
{
	this->settingsHandle = CreateDialog( GetModuleHandle( NULL ), 
								  MAKEINTRESOURCE( IDD_DIALOG1 ), 
								  this->GetHandle(), 
								  reinterpret_cast<DLGPROC>( TextWindow::dlgProc ) );
	ShowWindow( this->GetSettingHandle(), SW_SHOW );
}

BOOL __stdcall TextWindow::dlgProc( HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam )  //По-другому не линкуется
{
	HWND parentHandle = ::GetParent(hWndDlg);
	TextWindow* wnd = reinterpret_cast< TextWindow* >( GetWindowLong( parentHandle, GWL_USERDATA) );			

	switch( Msg ) {
		case WM_INITDIALOG:
			wnd->onSettingsInit( hWndDlg );
			return TRUE;
		case WM_COMMAND:
			switch( LOWORD( wParam ) ) {
				case IDOK:
					wnd->onSaveNewSettings();
					EndDialog( hWndDlg, LOWORD( wParam ) );
					break;
				case IDCANCEL:
					wnd->onRestoreOldSettings();
					EndDialog( hWndDlg, LOWORD( wParam ) );
					break;
				case IDC_BUTTON2:
					wnd->onSettingsFontColor( hWndDlg );
					return TRUE;
				case IDC_BUTTON3:
					wnd->onSettingsBgColor( hWndDlg );
					return TRUE;
				case IDC_CHECK1:
					wnd->onSettingsPreview( hWndDlg );
					return TRUE;
			}
			break;
		case WM_HSCROLL:
			wnd->onSettingsHScroll( hWndDlg );
			return TRUE;
	}
	return FALSE;
}

void TextWindow::onSettingsFontColor( HWND hWndDlg )
{
	CHOOSECOLOR chooseColor = initChooseColor( hWndDlg );
	if( ::ChooseColor( &chooseColor ) ) { 
		if( ::SendDlgItemMessage( hWndDlg, IDC_CHECK1, BM_GETCHECK, 0, 0) ) {
			previewSettings.SetFontColor( chooseColor.rgbResult );
			this->reDrawTextEditControl();
		} else {
			previewSettings.SetFontColor( chooseColor.rgbResult );
		}
	}
}

CHOOSECOLOR TextWindow::initChooseColor(HWND pHandle) {
	CHOOSECOLOR chooseColor;
	::ZeroMemory(&chooseColor, sizeof(CHOOSECOLOR));
	chooseColor.lStructSize = sizeof(CHOOSECOLOR);
	chooseColor.hwndOwner = pHandle;
	chooseColor.Flags = CC_ANYCOLOR;
	COLORREF cust_colors[16] = { 0 };
	chooseColor.lpCustColors = cust_colors;
	return chooseColor;
}

void TextWindow::onSettingsBgColor( HWND hWndDlg )
{
	CHOOSECOLOR chooseColor = initChooseColor( hWndDlg );
	if( ::ChooseColor( &chooseColor ) ) { 
		if( ::SendDlgItemMessage( hWndDlg, IDC_CHECK1, BM_GETCHECK, 0, 0) ) {
			previewSettings.SetBgColor( chooseColor.rgbResult );
			this->reDrawTextEditControl();
		} else {
			previewSettings.SetBgColor( chooseColor.rgbResult );
		}
	}

}

void TextWindow::onSettingsPreview( HWND hWndDlg )
{
	if( ::SendDlgItemMessage( hWndDlg, IDC_CHECK1, BM_GETCHECK, 0, 0) ) {
		previewSettings.SetTransparency( ::SendDlgItemMessage( hWndDlg, IDC_SLIDER2, TBM_GETPOS, 0, 0 ) );
		previewSettings.SetFontSize( ::SendDlgItemMessage( hWndDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0 ) );		
		this->updateSettings();
	}

}

void TextWindow::onSettingsInit( HWND h )
{
	this->previewSettings = this->mainSettings; 
	::SendDlgItemMessage( h, IDC_SLIDER2, TBM_SETRANGE, true, MAKELONG( 0, 255 ) );
	::SendDlgItemMessage( h, IDC_SLIDER2, TBM_SETPOS, true, previewSettings.GetTransparency() );
	::SendDlgItemMessage( h, IDC_SLIDER1, TBM_SETRANGE, true, MAKELONG( 8, 72 ) );
	::SendDlgItemMessage( h, IDC_SLIDER1, TBM_SETPOS, true, previewSettings.GetFontSize() );	
}

void TextWindow::onSettingsHScroll( HWND hWndDlg )
{
	if( SendDlgItemMessage( hWndDlg, IDC_CHECK1, BM_GETCHECK, 0, 0 ) ) {
		previewSettings.SetTransparency( ::SendDlgItemMessage( settingsHandle, IDC_SLIDER2, TBM_GETPOS, 0, 0 ) );
		previewSettings.SetFontSize( ::SendDlgItemMessage( settingsHandle, IDC_SLIDER1, TBM_GETPOS, 0, 0 ) );
		this->updateSettings();
	} else {
		previewSettings.SetTransparency( ::SendDlgItemMessage( settingsHandle, IDC_SLIDER2, TBM_GETPOS, 0, 0 ) );
		previewSettings.SetFontSize( ::SendDlgItemMessage( settingsHandle, IDC_SLIDER1, TBM_GETPOS, 0, 0 ) );
	}
}

void TextWindow::onSaveNewSettings()
{
	this->mainSettings = this->previewSettings;
	updateSettings();
	this->reDrawTextEditControl();
	settingsHandle = NULL;
}

void TextWindow::onRestoreOldSettings()
{
	this->previewSettings = this->mainSettings;
	updateSettings();
//	this->reDrawTextEditControl();
	settingsHandle = NULL;
}

void TextWindow::updateSettings( ) 
{
	 HWND hWndDlg = settingsHandle;
	::SetLayeredWindowAttributes( handle, RGB(0, 0, 0), previewSettings.GetTransparency(), LWA_ALPHA );
	HDC hdc = ::GetDC( this->GetEditHandle() );
	LOGFONT logFont;
	::GetObject( previewSettings.GetFont(), sizeof(LOGFONT), &logFont );
	logFont.lfHeight = -::MulDiv( previewSettings.GetFontSize(), ::GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
	::ReleaseDC( this->GetEditHandle(), hdc );
	HFONT hFont = ::CreateFontIndirect( &logFont );
	::SendMessage( editHandle, WM_SETFONT, reinterpret_cast<WPARAM>( hFont ), true );
	::DeleteObject( previewSettings.GetFont() );
	previewSettings.SetFont( hFont );
	this->reDrawTextEditControl();
}

void TextWindow::reDrawTextEditControl() {
	RECT clientRect;
	::GetClientRect( this->GetEditHandle(), &clientRect );
	::InvalidateRect( this->GetEditHandle(), &clientRect, TRUE );
}

bool TextWindow::Create( std::wstring className, HINSTANCE hInstance) 
{
	this->wndTitle = new wchar_t[10];
	LoadString( hInstance, 105, this->wndTitle, 10 );
	handle = CreateWindowEx(
		WS_EX_LAYERED,
		className.c_str(),
		wndTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, this );
	return handle != NULL;
}

void TextWindow::Show( int cmdShow )
{
	HACCEL haccel = LoadAccelerators( GetModuleHandle( NULL ), L"MY_ACC" ); 
	if( haccel == NULL ) {
		std::cerr << "Error of loanding accelerators";
	}
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

void TextWindow::onDestroy()
{
	PostQuitMessage( 0 );
}

void TextWindow::onClose() 
{
	if( HasChangedTextInBox() ) {
		int isSave, isExit = MessageBox( this->GetHandle(), L"Do you really want to exit", L"Attention!", MB_YESNO );
		bool succSave; 
		switch( isExit ) {
			case 6: //yes - continue
				isSave = MessageBox( this->GetHandle(), L"Do you want to save the text?", L"Save", MB_YESNOCANCEL );
				switch( isSave ) {
					case 6: //yes - save
						succSave = onSave();
						if( succSave ) {
							DestroyWindow( handle ); 
						} else {
							MessageBox( this->GetHandle(), L"File wasn't saved", L"Save", MB_OK );
						}
						break;
					case 7: //no - exit
						DestroyWindow( handle ); 
						break;
					case 2: //cancel - continue
						break;				
				}
				break;
			case 7: //no - continue, так можно писать?
				break;
		}
	} else {
		DestroyWindow( handle ); 
	}
}

HWND TextWindow::GetHandle() const
{
	return handle;
}

HWND TextWindow::GetEditHandle() const
{
	return this->editHandle;
}

HWND TextWindow::GetSettingHandle() const 
{
	return this->settingsHandle;
}

bool TextWindow::HasChangedTextInBox() const
{
	return this->isTextInBox;
}

void TextWindow::onCreate( RECT rect ) 
{
	this->editWnd = new EditCtrlWindow();
	editWnd->Create( GetModuleHandle( NULL ), handle, 997 );
	this->SetEditHadle( editWnd->GetHandle() );
	::SetLayeredWindowAttributes( handle, RGB(0, 0, 0), mainSettings.GetTransparency(), LWA_ALPHA );
	DWORD res = GetLastError();
	LOGFONT logFont;
	GetObject( mainSettings.GetFont(), sizeof( logFont ), &logFont );
	HDC hdc = ::GetDC( this->GetEditHandle() );
	logFont.lfHeight = -::MulDiv( this->mainSettings.GetFontSize(), GetDeviceCaps ( hdc, LOGPIXELSY ), 72 );
	::ReleaseDC( editHandle, hdc );
	HFONT hFont = ::CreateFontIndirect(&logFont);
	DeleteObject( this->mainSettings.GetFont() );
	this->mainSettings.SetFont( hFont );
	::SendMessage( this->GetEditHandle(), WM_SETFONT, reinterpret_cast<WPARAM>( hFont ), true );
	::SendMessage( this->GetEditHandle(), WM_SETTEXT, 0, reinterpret_cast<LPARAM>( this->initText ) ); 
}

void TextWindow::onResize()
{
	RECT rect;
	GetClientRect( this->GetHandle(), &rect );
	MoveWindow( this->GetEditHandle(), 0, 0, rect.right, rect.bottom, TRUE );
	editWnd->Show( SW_SHOW );
}

bool TextWindow::onSave()
{
	try {
		OPENFILENAME filePtr;
		filePtr.lStructSize = sizeof( OPENFILENAME ); 
		filePtr.hwndOwner = this->GetHandle(); 
		filePtr.hInstance = reinterpret_cast<HINSTANCE>( GetWindowLong( this->GetHandle(), GWL_HINSTANCE ) );
		filePtr.lpstrFilter = 0; //фильтры какие файлы можно юзать
		filePtr.lpstrCustomFilter = (LPWSTR)NULL; //фильтры выбранные пользвателем
		filePtr.nMaxCustFilter = 0; //размер кастомных фильтров?
		filePtr.nFilterIndex = 0; //индекс выбранного сейчас фильтра
		filePtr.lpstrFile = new wchar_t[ 100 ]; //имя файла в edit contol
		for( int i = 0; i <= 100; i++) {
			filePtr.lpstrFile[i] = '\0';
		}
		filePtr.nMaxFile = 100; //размер имени файла
		filePtr.lpstrFileTitle = 0; //выбранный файл
		filePtr.nMaxFileTitle = 0;  //его размер
		filePtr.lpstrInitialDir = (LPWSTR) NULL; //где 
		filePtr.lpstrTitle = L"SaveAs"; //
		filePtr.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT; 
		filePtr.nFileOffset = 0; //сколько символов до мени
		filePtr.nFileExtension = 0; //сколько символов до расширения
		filePtr.lpstrDefExt = L"txt";  //указатель на расширение по умолчанию
		BOOL isSuccess = GetSaveFileNameW( &filePtr );
		DWORD isError = CommDlgExtendedError();
		if( ( isSuccess == 0 ) && ( isError ) ) {
			throw std::runtime_error( "Cant instantiate save window" );
		} else
		if( ( isSuccess != 0 ) ) {			
			this->saveToFile( filePtr.lpstrFile );
			return true;
		}
		return false;
	}
	catch ( std::exception exp ) {
		std::cerr << exp.what();
		return false;
	}
}

void TextWindow::saveToFile( LPCTSTR fileName ) 
{
	HANDLE hFile = CreateFile( fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) {
		throw std::runtime_error( "unpredictable behavior" );
	} else {
		int len = 0;
		wchar_t* text = this->GetEditCtrlWndText( len );
		this->writeToFile( hFile, text, len );
		delete text;
	}
	CloseHandle( hFile );
}

void TextWindow::writeToFile( HANDLE hFile, wchar_t* text, const int len ) 
{
	DWORD dwBytesToWrite = static_cast<DWORD>( len * sizeof(wchar_t) );
	DWORD dwBytesWritten = 0;
	BOOL errorFlag = WriteFile( hFile, text, dwBytesToWrite, &dwBytesWritten, NULL ); // тут может вывалиться куча ошибок но из туториала непонятно как обработать все
	// не нашел что делать, если записал не все и откуда взять данные об ошибке GetLastError вернет только что за ошибка.
	if( errorFlag == FALSE ) {
		throw std::runtime_error( "Error while writing to file" );
	} else
	if( dwBytesWritten != dwBytesToWrite ) {
		throw std::runtime_error( "Error: dwBytesWritten != dwBytesToWrite" );
	}
}

wchar_t* TextWindow::GetEditCtrlWndText( int &len ) const
{
	int textLen = SendMessage( this->GetEditHandle(), WM_GETTEXTLENGTH, 0, 0 );
	if( textLen == -1 ) {
		int result = MessageBox( this->GetHandle(), L"No text to process", L"Attention!", MB_OK );
		throw std::length_error( "Error while getting text length" );
	} else {
		wchar_t* text = new wchar_t[ textLen + 1 ];
		int copyVal = SendMessage( this->GetEditHandle(), WM_GETTEXT, textLen + 1, reinterpret_cast<LPARAM>( text ) ); //Edit_GetText()?
		if ( copyVal == textLen ) {
			len = textLen;
			return text;
		} else {
			throw std::length_error( "Returned text length is not equal to current" );
		}
	}	 
}

LRESULT TextWindow::onOnCTLColorEdit( WPARAM wParam, LPARAM lParam )
{
	HDC hdc = reinterpret_cast<HDC>( wParam );
	HWND handle = reinterpret_cast<HWND>( lParam );
	::SetBkColor( hdc, previewSettings.GetBgColor() );
	::SetDCBrushColor( hdc, previewSettings.GetBgColor() );
	::SetTextColor( hdc, previewSettings.GetFontColor() );
	return reinterpret_cast<LRESULT>( ::GetStockObject( DC_BRUSH ) );
}